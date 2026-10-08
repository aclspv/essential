#include "./load.h"
#include "./util.h"

#include "./lbl.h"
#include "./common.h"
#include "./ssa.h"

#include <ae2f/pat/dbg.h>
#include <util/log.h>

#include <aclty.h>

/* ================================================================== */
/* SSA value flow for non-explicit accesses.                          */
/*                                                                    */
/* a non-explicit load is a read of the register's *value* in the     */
/* control flow, not of its storage. the value on entry to a block    */
/* is: the last store inside the block, else the value of its         */
/* (single) predecessor's exit, else a phi over the predecessor       */
/* exits when they diverge, else the interface seed (the ordinary     */
/* OpLoad in the root block). stores along the way are pure SSA       */
/* definitions: they emit no OpStore; the phi chains emit one OpPhi   */
/* at the head of their block (see emit.c).                           */
/* ================================================================== */

/** marker: no single SSA value exists (root seed / unresolved cycle):
 * keep the ordinary memory load. */
#define	ACLCUR_SSA_NONE		((aclspv_id_t)0xFFFFFFFE)

/** instant-predecessor capacity of one block. */
#define	ACLCUR_SSA_MAX_PRED	64

/** has c_reg ever been the target of a store anywhere in this function?
 *
 * whole-buffer scan, the same shape as aclcur_get_node_store's per-block one.
 * the store node records the RESOLVED register (aclcur_store_reg resolves
 * before recording), which is what c_reg already is at the call site.
 *
 * this exists so the unassigned-value rejection does not inherit the
 * resolver's blind spots: it has three silent caps — more than
 * ACLCUR_SSA_MAX_PRED predecessors, a dead label, and its own depth bound —
 * and every one of them reports "no value". under
 * ACLSPV_CTX_FLAG0_ERROR_UNASSIGNED_VAL_USED that would turn a correctly
 * assigned variable into a hard error on a large or deep function. asking
 * whether a store exists at all is answerable regardless. */
static aclspv_bool_t	s_reg_ever_stored(
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	aclspv_wrd_t i;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t q = &s_get_buf(h_ctx)[i];

		if(	q->m_class == ACLCUR_STORE
			&&	q->m_d.m_store.m_reg == c_reg)
			return 1;
	}

	return 0;
}

static aclspv_id_t	s_ssa_entry_i(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		aclspv_id_t c_root,
		h_aclspv_ctx_t h_ctx,
		aclspv_wrd_t c_depth
		);

/** head of the loop-header authoritative chain serving c_reg (0: none). */
static aclspv_id_t	s_ssa_loop_head(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		);

/** create a value-flow phi chain for c_reg at c_lbl: one node per
 * (value, label) entry, marked ACLCUR_FLAG0_PHI_SELF (and, iff c_auth,
 * ACLCUR_FLAG0_PHI_AUTH so the post-carve carry fill completes its
 * back-edge operands). returns the chain head. */
static aclspv_id_t	s_ssa_chain_create(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx,
		const aclspv_id_t* ae2f_restrict r_vals,
		const aclspv_id_t* ae2f_restrict r_plbs,
		aclspv_wrd_t c_nv,
		aclspv_bool_t c_auth
		);

/** label of the block that owns the cursor at idx. */
static aclspv_id_t	s_ssa_block_lbl(aclspv_wrd_t idx, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t j = idx;

	while(j) {
		h_aclcur_t q = &s_get_buf(h_ctx)[--j];
		if(q->m_class == ACLCUR_LBL)
			return q->m_id;
	}
	return 0;
}

/** immediate predecessors of c_lbl, deduplicated, in cursor order.
 * 0 return: none, or the predecessor buffer overflowed (the caller then
 * treats the block as an interface root and falls back to memory). */
static aclspv_wrd_t	s_ssa_preds(
		aclspv_id_t c_lbl,
		aclspv_id_t* ae2f_restrict r_pds,
		aclspv_wrd_t c_max,
		h_aclspv_ctx_t h_ctx
		) {
	aclspv_wrd_t n = 0;
	aclspv_wrd_t i;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t q = &s_get_buf(h_ctx)[i];
		aclspv_bool_t hit = 0;

		switch(q->m_class) {
			case ACLCUR_J:
				hit = (q->m_d.m_j.m_lbl == c_lbl);
				break;
			case ACLCUR_JC:
				hit = (q->m_d.m_jc.m_lbl_true == c_lbl
						|| q->m_d.m_jc.m_lbl_false == c_lbl);
				break;
			default:
				break;
		}

		if(!hit)
			continue;

		{
			aclspv_id_t p = s_ssa_block_lbl(i, h_ctx);
			aclspv_wrd_t j;

			if(!p)
				continue;
			for(j = 0; j < n; ++j)
				if(r_pds[j] == p)
					break;
			if(j == n) {
				if(n >= c_max)
					return 0;
				r_pds[n++] = p;
			}
		}
	}

	return n;
}

/** the label is emit-live: its instructions actually land in the binary.
 * the emission sweep powers off and skips every label whose reference
 * count cascades to zero. a label is dead with certainty when it has no
 * incoming edges, and equally dead when every surviving reference comes
 * from dead labels — follow the predecessor edges up to the root. a
 * value served from a dead block would dangle (its definition is
 * elided), so dead predecessors contribute neither stores nor exit
 * values to the resolution. */
static aclspv_bool_t s_ssa_lbl_live(
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx,
		aclspv_id_t* ae2f_restrict p_vis,
		aclspv_wrd_t c_nvis,
		aclspv_wrd_t c_depth
		) {
	h_aclcur_t pl = aclcur_get_node(c_lbl, h_ctx);
	aclspv_id_t pds[ACLCUR_SSA_MAX_PRED];
	aclspv_wrd_t i;

	if(!pl || pl->m_class != ACLCUR_LBL || c_depth > 256)
		return 0;

	if(pl->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_ROOT))
		return 1;

	/** no incoming edges: powered off at emission. */
	unless(pl->m_num_ref)
		return 0;

	/** visited: a cycle yields nothing unless another, outer edge does. */
	for(i = 0; i < c_nvis; ++i)
		if(p_vis[i] == c_lbl)
			return 0;
	if(c_nvis >= ACLCUR_SSA_MAX_PRED)
		return 0;
	p_vis[c_nvis] = c_lbl;

	/** alive iff at least one of its predecessors is, recursively. */
	{
		aclspv_wrd_t np = s_ssa_preds(c_lbl, pds, ACLCUR_SSA_MAX_PRED, h_ctx);

		unless(np)
			return 0;

		for(i = 0; i < np; ++i)
			if(s_ssa_lbl_live(pds[i], h_ctx, p_vis, c_nvis + 1, c_depth + 1))
				return 1;
	}

	return 0;
}

/** exported entry: a fresh visited set per query. */
aclspv_bool_t aclcur_ssa_lbl_live(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx) {
	aclspv_id_t vis[ACLCUR_SSA_MAX_PRED];

	return s_ssa_lbl_live(c_lbl, h_ctx, vis, 0, 0);
}

/** value of c_reg on entry to c_lbl: a single SSA result id, or
 * ACLCUR_SSA_NONE when only a memory load can serve it. */
static aclspv_id_t	s_ssa_entry_i(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		aclspv_id_t c_root,
		h_aclspv_ctx_t h_ctx,
		aclspv_wrd_t c_depth
		) {
	aclspv_id_t pds[ACLCUR_SSA_MAX_PRED];
	aclspv_id_t vals[ACLCUR_SSA_MAX_PRED];
	aclspv_id_t plbs[ACLCUR_SSA_MAX_PRED];
	aclspv_id_t v0;
	aclspv_wrd_t np;
	aclspv_wrd_t nv = 0;
	aclspv_wrd_t i;

	/** recursion re-entering the block whose entry is being resolved is a
	 * loop back edge: the carried value is constant around the loop (a
	 * store inside the loop would have been found before this recursion). */
	if(c_depth && c_lbl == c_root)
		return ACLCUR_SSA_NONE;

	/** a loop header's entry value is its authoritative chain: a read at
	 * the header created it (ACLCUR_FLAG0_PHI_AUTH), the post-carve
	 * carry fill completes the back-edge operands. serve it to any
	 * resolution that reaches the header — the body's reads over the
	 * forward edge, the carry fill on the final graph — instead of the
	 * stale pre-loop value the partially-carved graph would resolve. */
	{
		h_aclcur_t p_h = aclcur_get_node(c_lbl, h_ctx);

		if(p_h
				&& p_h->m_class == ACLCUR_LBL
				&& (p_h->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_LOOP))) {
			aclspv_id_t head = s_ssa_loop_head(c_reg, c_lbl, h_ctx);

			/** a chain already serves the register. */
			if(head)
				return head;

			/** a register first read in the loop BODY (never at the
			 * header) has no chain yet: create it here, at the header.
			 * its value flows over the forward edge into the loop and
			 * the phi carries it. the create-time entry is computed
			 * exactly like s_ssa_loop_head's determinism — the store
			 * walk of the forward (earliest-carved) predecessor — so
			 * the post-carve carry fill identifies this chain by its
			 * head operand and completes the back-edge entry; the
			 * chain is marked ACLCUR_FLAG0_PHI_AUTH for that reason.
			 * a register whose forward-edge value does not resolve
			 * (globals / params / unresolved cycles) keeps the
			 * memory-seed fallthrough below. */
			{
				aclspv_id_t f_pds[ACLCUR_SSA_MAX_PRED];
				aclspv_id_t f_vals[1];
				aclspv_id_t f_plbs[1];
				aclspv_wrd_t f_np = s_ssa_preds(
						c_lbl
						, f_pds
						, ACLCUR_SSA_MAX_PRED
						, h_ctx
						);

				if(f_np) {
					aclspv_id_t f_p0 = f_pds[0];
					aclspv_id_t f_v0 = 0;
					h_aclcur_t p_st;
					aclspv_id_t id_st = aclcur_get_node_store(c_reg, f_p0, h_ctx);

					if(id_st) {
						p_st = aclcur_get_node(id_st, h_ctx);
						f_v0 = p_st ? p_st->m_val : 0;
					}
					if(!f_v0)
						f_v0 = s_ssa_entry_i(c_reg, f_p0, f_p0, h_ctx, 0);

					if(f_v0 && f_v0 != ACLCUR_SSA_NONE) {
						f_vals[0]	= f_v0;
						f_plbs[0]	= f_p0;
						head = s_ssa_chain_create(
								c_reg
								, c_lbl
								, h_ctx
								, f_vals
								, f_plbs
								, 1
								, 1
								);
						if(head)
							return head;
					}
				}
			}
		}
	}

	/** safety net for irreducible cycles: deeper loops fail toward the
	 * memory seed instead of recursing forever. */
	if(c_depth > h_ctx->m_num_cursor + 8)
		return ACLCUR_SSA_NONE;

	np = s_ssa_preds(c_lbl, pds, ACLCUR_SSA_MAX_PRED, h_ctx);
	if(!np)
		return ACLCUR_SSA_NONE;

	for(i = 0; i < np; ++i) {
		aclspv_id_t e;
		aclspv_id_t vis[ACLCUR_SSA_MAX_PRED];

		/** dead predecessors contribute no value: their stores and exit
		 * values are elided at emission and would dangle here. */
		unless(s_ssa_lbl_live(pds[i], h_ctx, vis, 0, 0))
			continue;

		e = aclcur_get_node_store(c_reg, pds[i], h_ctx);

		if(e) {
			h_aclcur_t p_store = aclcur_get_node(e, h_ctx);
			e = p_store ? p_store->m_val : 0;
		}

		/** no store inside the predecessor: its entry value flows out. */
		if(!e)
			e = s_ssa_entry_i(c_reg, pds[i], c_root, h_ctx, c_depth + 1);

		if(e == ACLCUR_SSA_NONE)
			continue;

		plbs[nv]	= pds[i];
		vals[nv]	= e;
		++nv;
	}

	if(!nv)
		return ACLCUR_SSA_NONE;

	/** a single value converges without a phi. */
	v0 = vals[0];
	for(i = 1; i < nv; ++i)
		if(vals[i] != v0)
			break;
	if(i == nv)
		return v0;

	/** divergent incoming values: converge at this block with a phi. */

	/** reuse a value-flow chain of this block with the same operands. */
	{
		aclspv_wrd_t j;

		for(j = 0; j < h_ctx->m_num_cursor; ++j) {
			h_aclcur_t q = &s_get_buf(h_ctx)[j];
			aclspv_id_t head;
			aclspv_wrd_t used[ACLCUR_SSA_MAX_PRED];
			aclspv_wrd_t nn = 0;
			aclspv_id_t idc;

			if(q->m_class != ACLCUR_PHI
					|| !(q->m_flag0 & aclcur_flag0_phi(ACLCUR_FLAG0_PHI_SELF))
					|| q->m_lbl != c_lbl)
				continue;

			head = aclcur_phi_head(q->m_id, h_ctx);
			ae2f_ifezerr(head)
				continue;

			idc = head;
			while(idc && nn < nv) {
				h_aclcur_t qp = aclcur_get_node(idc, h_ctx);
				aclspv_wrd_t k;
				aclspv_bool_t found = 0;

				for(k = 0; k < nv && !found; ++k) {
					aclspv_wrd_t m;

					if(qp->m_d.m_phi.m_val != vals[k]
							|| qp->m_d.m_phi.m_lbl != plbs[k])
						continue;
					for(m = 0; m < nn; ++m)
						if(used[m] == k)
							break;
					found = (m == nn);
					if(found)
						used[nn++] = k;
				}

				if(!found)
					break;
				idc = qp->m_d.m_phi.m_nxt;
			}

			if(idc == 0 && nn == nv)
				return head;
		}
	}

	/** create the chain: one phi node per (value, label) operand. */
	{
		aclspv_id_t head = 0;
		aclspv_id_t prv = 0;
		h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);
		aclspv_id_t ty = p_reg ? p_reg->m_ty : 0;

		for(i = 0; i < nv; ++i) {
			aclspv_id_t id_phi = aclcur_cfg_node2(0, h_ctx, 1);
			h_aclcur_t p_phi;

			ae2f_ifezerr(id_phi)
				return ACLCUR_SSA_NONE;

			/** cfg_node2 reallocates and frees the cursor buffer: re-fetch
			 * every pointer that predates the call. */
			p_phi = aclcur_get_node(id_phi, h_ctx);

			p_phi->m_class		= ACLCUR_PHI;
			p_phi->m_ty		= ty;
			p_phi->m_val		= id_phi;
			p_phi->m_lbl		= c_lbl;
			p_phi->m_d.m_phi.m_lbl	= plbs[i];
			p_phi->m_d.m_phi.m_val	= vals[i];
			p_phi->m_d.m_phi.m_nxt	= 0;
			p_phi->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE)
				| aclcur_flag0_phi(ACLCUR_FLAG0_PHI_SELF);

			if(prv) {
				h_aclcur_t p_prv = aclcur_get_node(prv, h_ctx);
				p_prv->m_d.m_phi.m_nxt = id_phi;
			} else {
				head = id_phi;
			}
			prv = id_phi;
		}

		return head;
	}
}

/** head of the loop-header authoritative chain that serves c_reg (the
 * chain whose first operand is the store-walk entry of c_reg from the
 * header's forward predecessor), 0: none. the phi nodes carry no
 * register id; the create-time entry recomputes deterministically, so
 * a chain is identified per register by its first (value, label). */
static aclspv_id_t	s_ssa_loop_head(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		) {
	aclspv_id_t pds[ACLCUR_SSA_MAX_PRED];
	aclspv_id_t v0 = 0;
	aclspv_id_t p0 = 0;
	aclspv_wrd_t np;
	aclspv_wrd_t j;

	np = s_ssa_preds(c_lbl, pds, ACLCUR_SSA_MAX_PRED, h_ctx);
	if(!np)
		return 0;

	/** the create-time predecessor is the earliest-carved one: the
	 * forward edge into the loop (the back edge carves after the body
	 * and the continue trampoline). */
	p0 = pds[0];

	/** the create-time entry value: the store walk of the forward
	 * predecessor. */
	{
		h_aclcur_t p_st;
		aclspv_id_t id_st = aclcur_get_node_store(c_reg, p0, h_ctx);

		if(id_st) {
			p_st = aclcur_get_node(id_st, h_ctx);
			v0 = p_st ? p_st->m_val : 0;
		}
		if(!v0)
			v0 = s_ssa_entry_i(c_reg, p0, p0, h_ctx, 0);
	}

	if(!v0 || v0 == ACLCUR_SSA_NONE)
		return 0;

	/** find the authoritative chain whose head carries the entry. */
	for(j = 0; j < h_ctx->m_num_cursor; ++j) {
		h_aclcur_t q = &s_get_buf(h_ctx)[j];
		aclspv_id_t head;

		if(q->m_class != ACLCUR_PHI
				|| !(q->m_flag0 & aclcur_flag0_phi(ACLCUR_FLAG0_PHI_AUTH))
				|| q->m_lbl != c_lbl)
			continue;

		head = aclcur_phi_head(q->m_id, h_ctx);
		ae2f_ifezerr(head)
			continue;

		{
			h_aclcur_t p_h = aclcur_get_node(head, h_ctx);

			if(p_h
					&& p_h->m_d.m_phi.m_lbl == p0
					&& p_h->m_d.m_phi.m_val == v0)
				return head;
		}
	}

	return 0;
}

/** create a value-flow phi chain for c_reg at c_lbl: one phi node per
 * (value, label) entry, marked ACLCUR_FLAG0_PHI_SELF (and, iff c_auth,
 * ACLCUR_FLAG0_PHI_AUTH so the post-carve carry fill completes its
 * back-edge operands). returns the chain head. */
static aclspv_id_t	s_ssa_chain_create(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx,
		const aclspv_id_t* ae2f_restrict r_vals,
		const aclspv_id_t* ae2f_restrict r_plbs,
		aclspv_wrd_t c_nv,
		aclspv_bool_t c_auth
		) {
	aclspv_id_t head = 0;
	aclspv_id_t prv = 0;
	h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);
	aclspv_id_t ty = p_reg ? p_reg->m_ty : 0;
	aclspv_wrd_t i;

	for(i = 0; i < c_nv; ++i) {
		aclspv_id_t id_phi = aclcur_cfg_node2(0, h_ctx, 1);
		h_aclcur_t p_phi;

		ae2f_ifezerr(id_phi)
			return 0;

		/** cfg_node2 reallocates and frees the cursor buffer:
		 * re-fetch every pointer that predates the call. */
		p_phi = aclcur_get_node(id_phi, h_ctx);

		p_phi->m_class		= ACLCUR_PHI;
		p_phi->m_ty		= ty;
		p_phi->m_val		= id_phi;
		p_phi->m_lbl		= c_lbl;
		p_phi->m_d.m_phi.m_lbl	= r_plbs[i];
		p_phi->m_d.m_phi.m_val	= r_vals[i];
		p_phi->m_d.m_phi.m_nxt	= 0;
		p_phi->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE)
			| aclcur_flag0_phi(ACLCUR_FLAG0_PHI_SELF)
			| (c_auth ? aclcur_flag0_phi(ACLCUR_FLAG0_PHI_AUTH) : 0);

		if(prv) {
			h_aclcur_t p_prv = aclcur_get_node(prv, h_ctx);

			p_prv->m_d.m_phi.m_nxt = id_phi;
		} else {
			head = id_phi;
		}
		prv = id_phi;
	}

	return head;
}

/** the read at a loop header rides an authoritative value-flow phi
 * chain instead of memory: one node per create-time predecessor (the
 * forward edge — the back edge is not carved yet), marked
 * ACLCUR_FLAG0_PHI_AUTH, served as the chain head. reads that resolve
 * to nothing (globals / params / unresolved cycles) return 0: they
 * keep the memory seed, a seed load in the header dominates the loop. */
static aclspv_id_t	s_ssa_loop_read(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_lbl = aclcur_get_lbl_last1(h_ctx);
	aclspv_id_t pds[ACLCUR_SSA_MAX_PRED];
	aclspv_id_t vals[ACLCUR_SSA_MAX_PRED];
	aclspv_id_t plbs[ACLCUR_SSA_MAX_PRED];
	aclspv_wrd_t np;
	aclspv_wrd_t nv = 0;
	aclspv_wrd_t i;

	ae2f_ifezerr(p_lbl)		return 0;
	if(!(p_lbl->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_LOOP)))
		return 0;

	/** a chain already serves the register: return its head. */
	{
		aclspv_id_t h = s_ssa_loop_head(c_reg, p_lbl->m_id, h_ctx);

		if(h)
			return h;
	}

	np = s_ssa_preds(p_lbl->m_id, pds, ACLCUR_SSA_MAX_PRED, h_ctx);
	if(!np)
		return 0;

	for(i = 0; i < np; ++i) {
		h_aclcur_t p_st;
		aclspv_id_t e = 0;
		aclspv_id_t id_st = aclcur_get_node_store(c_reg, pds[i], h_ctx);

		/** the entry value: the create-time predecessor's store, else
		 * its entry value (the existing store walk). */
		if(id_st) {
			p_st = aclcur_get_node(id_st, h_ctx);
			e = p_st ? p_st->m_val : 0;
		}
		if(!e)
			e = s_ssa_entry_i(c_reg, pds[i], pds[i], h_ctx, 0);

		if(!e || e == ACLCUR_SSA_NONE)
			continue;

		vals[nv]	= e;
		plbs[nv]	= pds[i];
		++nv;
	}

	/** the read does not resolve: the memory seed serves it. */
	if(!nv)
		return 0;

	/** create the authoritative chain: one phi node per entry. */
	return s_ssa_chain_create(
			c_reg
			, p_lbl->m_id
			, h_ctx
			, vals
			, plbs
			, nv
			, 1
			);
}

/** value of c_reg on entry of the current block, or ACLCUR_SSA_NONE. */
static aclspv_id_t	s_ssa_entry(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_lbl = aclcur_get_lbl_last1(h_ctx);

	ae2f_ifezerr(p_lbl)
		return ACLCUR_SSA_NONE;

	return s_ssa_entry_i(c_reg, p_lbl->m_id, p_lbl->m_id, h_ctx, 0);
}

static aclspv_id_t	s_load_without_phi(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx,
		aclutil_literal* ae2f_restrict r_lit_opt,
		h_aclcur_t* ae2f_restrict	r_new_load,
		const aclspv_bool_t c_explicit
		) {
	h_aclcur_t p = aclcur_get_node(c_reg, h_ctx);
	ae2f_ifezerr(p) return 0;
	ae2fpat_assertassume_strict(p->m_class == ACLCUR_REG);

	if((p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED))
			&& !c_explicit) {
		if(r_lit_opt && p->m_val_mask & ACLUTIL_LITERAL_MASK_LITERAL)
			*r_lit_opt = p->m_val_lit;

		return p->m_val;
	}

	/** 
	 * loading is required to be marked in this stage.
	 * that means in here, p->m_val SHOULD BE zero.
	 * */
	ae2f_ifnezerr(p->m_val) return 0;

	/** non-explicit: the register's value is a property of the control
	 * flow, not of its storage. resolve the value entering the current
	 * block: a single value serves directly, a divergent join converges
	 * in a phi, and only the interface seeds (params / globals, first
	 * touched by a load) keep the ordinary OpLoad.
	 *
	 * the loop header's reads ride an authoritative phi chain instead:
	 * the value enters the loop once (over the forward edge), then the
	 * phi carries it — the post-carve fill appends the back-edge
	 * operands, so the loop emits no OpLoad/OpStore of its carried
	 * registers. reads that resolve to nothing keep the memory seed: a
	 * seed load in the header dominates the whole loop. */
	unless(c_explicit) {
		h_aclcur_t p_lbl = aclcur_get_lbl_last1(h_ctx);
		aclspv_id_t id_ssa = 0;

		if(p_lbl && (p_lbl->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_LOOP)))
			id_ssa = s_ssa_loop_read(c_reg, h_ctx);
		else
			id_ssa = s_ssa_entry(c_reg, h_ctx);

		if(id_ssa && id_ssa != ACLCUR_SSA_NONE) {
			h_aclcur_t p_b;

			/** the resolution may have grown the cursor buffer:
			 * re-fetch the register before touching it. */
			p = aclcur_get_node(c_reg, h_ctx);
			p_b = aclcur_get_node(id_ssa, h_ctx);

			p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
			p->m_val	= id_ssa;
			p->m_val_mask	= 0;

			if(r_lit_opt && p_b
					&& (p_b->m_val_mask & ACLUTIL_LITERAL_MASK_LITERAL))
				*r_lit_opt = p_b->m_val_lit;

			return id_ssa;
		}

		/** no graph value: the memory seed load below serves it.
		 *
		 * ACLSPV_CTX_FLAG0_ERROR_UNASSIGNED_VAL_USED makes that an explicit
		 * rejection instead. With the flag off the seed load proceeds, and
		 * s_load_reg tracks its result for the rest of the branch, so the
		 * value is evaluated once and reused rather than reloaded.
		 *
		 * two exclusions, both load-bearing rather than cosmetic:
		 *
		 * - STORAGE CLASS. A seeded parameter or interface variable reaches
		 *   this fall-through legitimately: its value IS this seed load, and
		 *   nothing else distinguishes it here — the TRACKED flag, m_val and
		 *   the resolver's answer are bit-identical to an unassigned local's.
		 *   args.c cannot produce a Function-storage parameter at all (it
		 *   reaches default: -> ACLSPV_COMPILE_STORAGE_CLASS_UNDEFINED),
		 *   while vardecl always passes Function, so this closes the class
		 *   rather than the individual cases. That invariant is
		 *   undocumented and load-bearing; keep it.
		 *
		 * - ARRAYS. Reading an array value is an artifact of the visit
		 *   cascade: simple.h value-loads the array register, and the
		 *   subscript carve then discards that value and re-derives the
		 *   register from the cursor. The array's real value is its own
		 *   address. Until that cascade stops loading it, an array must not
		 *   be reported unassigned.
		 *
		 * note what this is NOT: a definite-assignment analysis. A register
		 * assigned on one arm and not another still resolves to its single
		 * stored value and is accepted. */
		{
			const aclty_t* const ae2f_restrict P_TY
				= aclty_find(p->m_ty, h_ctx);

			if(	(h_ctx->m_usrcfg.m_flag0
						& ACLSPV_CTX_FLAG0_ERROR_UNASSIGNED_VAL_USED)
				&&	p->m_d.m_reg.m_storage_class == SpvStorageClassFunction
				&&	!(P_TY && P_TY->m_class == ACLTY_ARR)
				&&	!s_reg_ever_stored(c_reg, h_ctx)
				) {
				err_puts("[aclcur::load] a variable is read before it "
						"is assigned, and no value reaches this branch");
				h_ctx->m_err = ACLSPV_COMPILE_UNASSIGNED_VALUE;
				return 0;
			}
		}
	}

	{
		aclspv_id_t load = aclcur_cfg_node(0, h_ctx);
		h_aclcur_t p_load = aclcur_get_node(load, h_ctx);
		h_aclcur_t p_lbl = aclcur_get_lbl_last1(h_ctx);

		p = aclcur_get_node(c_reg, h_ctx);

		ae2f_ifezerr(p_load)				return 0;
		ae2f_ifezerr(p)					return 0;
		ae2f_ifezerr(p_lbl)				return 0;

		p_load->m_class			= ACLCUR_LOAD;
		p_load->m_ty			= p->m_ty;
		p_load->m_num_ref		= 0;
		p_load->m_d.m_load.m_reg	= c_reg;
		p_load->m_lbl			= p_lbl->m_id;
		p_load->m_flag0 |= aclcur_flag0(ACLCUR_FLAG0_VALUE);
		if(c_explicit)
			p_load->m_flag0 |= aclcur_flag0_load(ACLCUR_FLAG0_LOAD_EXPLICIT);
		p_load->m_val			= p_load->m_id;

		(void)++p->m_num_ref;

		*r_new_load = p_load;
		return p_load->m_id;
	}

	ae2f_unreachable();
}

#include "./lbl/dfs.h"
#include "./store.h"
#include "./phi.h"

typedef struct {
	h_aclspv_ctx_t	m_ctx;
	aclspv_id_t	m_lbl;
	aclspv_id_t	m_reg;
	aclspv_id_t	m_load;
} s_load_phi_t;

static enum ACLCUR_LBL_DFS_ s_load_phi(
		h_aclcur_t		h_child,
		ae2f_unused h_aclcur_t	h_par_opt,
		void* ae2f_restrict	rdwr_client
		) {
	const s_load_phi_t* ae2f_restrict p = rdwr_client;
	aclspv_id_t	store;

	if(h_child->m_lbl == p->m_lbl)
		return ACLCUR_LBL_DFS_IN;

	unless(aclcur_lbl_is_reachable_uc(
				h_child->m_lbl
				, p->m_lbl
				, p->m_ctx))
		return ACLCUR_LBL_DFS_IN;

	unless((store = aclcur_get_node_store(
					p->m_reg
					, h_child->m_lbl
					, p->m_ctx
					))) return ACLCUR_LBL_DFS_IN;

	aclcur_cfg_load_phi(
			p->m_load
			, h_child->m_lbl
			, p->m_ctx);

	return ACLCUR_LBL_DFS_IN;
}

static aclspv_id_t	s_load_reg(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx,
		aclutil_literal* ae2f_restrict r_lit_opt,
		const aclspv_bool_t c_explicit
		) {
	h_aclcur_t	p_load = 0;
	h_aclcur_t	p_lbl;
	h_aclcur_t	p_reg;
	aclspv_id_t id_val;
	aclspv_id_t c_ref;
	s_load_phi_t phi;

	/** the caller may have passed an alias: it is referenced, too. */
	c_ref	= c_reg;

	/** alias register has no storage: load what it really is. */
	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)	return 0;

	/** 
	 * value for current branch 
	 * */
	id_val = s_load_without_phi(
			c_reg
			, h_ctx
			, r_lit_opt
			, &p_load
			, c_explicit
			);

	/** loading is not required. */
	ae2f_ifezerr(p_load)
		return id_val;

	/** the alias the load went through is a reference on it, too. */
	if(c_ref != c_reg)
		(void)++aclcur_get_node(c_ref, h_ctx)->m_num_ref;

	p_load->m_flag0 |= aclcur_flag0(ACLCUR_FLAG0_VALUE);


	ae2f_ifezerr(p_lbl = aclcur_get_node(
				p_load->m_lbl
				, h_ctx
				)) return 0;

	phi.m_lbl	= p_load->m_lbl;
	phi.m_reg	= c_reg;
	phi.m_ctx	= h_ctx;
	phi.m_load	= p_load->m_id;

	/** 
	 * register phi by label.
	 * provided phi was actually required, turn ACLCUR_FLAG0_OFF_REG_CHANGED on
	 * */
	aclcur_lbl_dfs(
			p_lbl->m_d.m_lbl.m_root
			, s_load_phi
			, &phi
			, h_ctx
		      );

	ae2f_ifezerr(p_reg = aclcur_get_node(c_reg, h_ctx))
		return 0;

	/** after load, value is tracked. */
	p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
	return p_reg->m_val = id_val;
}

aclspv_id_t	aclcur_load_reg(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx,
		aclutil_literal* ae2f_restrict r_lit_opt
		) {
	return s_load_reg(c_reg, h_ctx, r_lit_opt, 0);
}

aclspv_id_t	aclcur_load_reg_explicit(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx,
		aclutil_literal* ae2f_restrict r_lit_opt
		) {
	return s_load_reg(c_reg, h_ctx, r_lit_opt, 1);
}

/** post-carve, pre-emit: complete the loop-header authoritative phi
 * chains (ACLCUR_FLAG0_PHI_AUTH) with the final-graph carries.
 *
 * the back edge is carved after the header's reads resolve, so each
 * chain carries only its forward-edge entry; this walks the final
 * immediate predecessors of every live loop header and appends the
 * missing edge entries, one per predecessor:
 *   carry(P)  = value of the chain's register on entry of P (on the
 *               final graph), else
 *               the header's last store of the register (its block-exit
 *               value), else
 *               the chain head itself (self-referential: the header
 *               dominates the back-edge predecessor).
 * the chain then holds exactly the header's final predecessors: the
 * OpPhi emitted by the block hoist is complete and val-valid. */
aclspv_compile_t aclcur_ssa_loop_carry_fill(h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t i, j;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t q = &s_get_buf(h_ctx)[i];
		aclspv_id_t lbl;

		if(q->m_class != ACLCUR_LBL
				|| !(q->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_LOOP))
				|| !aclcur_ssa_lbl_live(q->m_id, h_ctx))
			continue;

		/** cfg_node2 appends below may realloc and free the cursor
		 * buffer: the label id is captured once and substituted for
		 * every later q deref. */
		lbl = q->m_id;

		for(j = 0; j < h_ctx->m_num_cursor; ++j) {
			h_aclcur_t qp = &s_get_buf(h_ctx)[j];
			aclspv_id_t head;
			aclspv_id_t reg = 0;
			aclspv_id_t pds[ACLCUR_SSA_MAX_PRED];
			aclspv_id_t cov[ACLCUR_SSA_MAX_PRED];
			aclspv_wrd_t npd;
			aclspv_wrd_t ncov = 0;
			aclspv_wrd_t k, m;

			if(qp->m_class != ACLCUR_PHI
					|| !(qp->m_flag0 & aclcur_flag0_phi(ACLCUR_FLAG0_PHI_AUTH))
					|| qp->m_lbl != lbl)
				continue;

			head = aclcur_phi_head(qp->m_id, h_ctx);
			ae2f_ifezerr(head)
				continue;

			/** the chain's register: the register whose create-time
			 * entry matches the chain head. */
			for(k = 0; k < h_ctx->m_num_cursor && !reg; ++k) {
				h_aclcur_t qr = &s_get_buf(h_ctx)[k];

				if(qr->m_class == ACLCUR_REG
						&& s_ssa_loop_head(qr->m_id, lbl, h_ctx) == head)
					reg = qr->m_id;
			}
			if(!reg)
				continue;

			/** the labels the chain already covers. */
			{
				aclspv_id_t idc = head;

				while(idc && ncov < ACLCUR_SSA_MAX_PRED) {
					h_aclcur_t qc = aclcur_get_node(idc, h_ctx);

					ae2f_ifezerr(qc)
						break;
					cov[ncov++] = qc->m_d.m_phi.m_lbl;
					idc = qc->m_d.m_phi.m_nxt;
				}
			}

			npd = s_ssa_preds(lbl, pds, ACLCUR_SSA_MAX_PRED, h_ctx);

			for(k = 0; k < npd; ++k) {
				aclspv_id_t carry = 0;
				aclspv_bool_t covered = 0;

				for(m = 0; m < ncov; ++m)
					if(cov[m] == pds[k]) {
						covered = 1;
						break;
					}
				if(covered)
					continue;

				/** the value on entry of the predecessor, on the final
				 * graph. */
				carry = s_ssa_entry_i(reg, pds[k], pds[k], h_ctx, 0);
				if(!carry || carry == ACLCUR_SSA_NONE) {
					h_aclcur_t p_st;
					aclspv_id_t id_st = aclcur_get_node_store(reg, lbl, h_ctx);

					/** the header's last store of the register: its
					 * block-exit value. */
					carry = 0;
					if(id_st) {
						p_st = aclcur_get_node(id_st, h_ctx);
						carry = p_st ? p_st->m_val : 0;
					}
				}

				/** still nothing: the chain head itself. */
				if(!carry || carry == ACLCUR_SSA_NONE)
					carry = head;

				/** append {carry, pred} at the chain tail. */
				{
					aclspv_id_t tail = head;
					aclspv_id_t id_phi;
					h_aclcur_t p_tail;
					h_aclcur_t p_phi;

					while(tail) {
						h_aclcur_t q_t = aclcur_get_node(tail, h_ctx);

						if(!q_t || !q_t->m_d.m_phi.m_nxt)
							break;
						tail = q_t->m_d.m_phi.m_nxt;
					}

					id_phi = aclcur_cfg_node2(0, h_ctx, 1);
					ae2f_ifezerr(id_phi)
						return ACLSPV_COMPILE_ALLOC_FAILED;

					/** cfg_node2 may have reallocated the buffer:
					 * re-fetch the chain pointers. */
					p_phi	= aclcur_get_node(id_phi, h_ctx);
					p_tail	= aclcur_get_node(tail, h_ctx);

					p_phi->m_class		= ACLCUR_PHI;
					p_phi->m_ty		= p_tail ? p_tail->m_ty : 0;
					p_phi->m_val		= id_phi;
					p_phi->m_lbl		= lbl;
					p_phi->m_d.m_phi.m_lbl	= pds[k];
					p_phi->m_d.m_phi.m_val	= carry;
					p_phi->m_d.m_phi.m_nxt	= 0;
					p_phi->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE)
						| aclcur_flag0_phi(ACLCUR_FLAG0_PHI_SELF)
						| aclcur_flag0_phi(ACLCUR_FLAG0_PHI_AUTH);

					if(p_tail)
						p_tail->m_d.m_phi.m_nxt = id_phi;
				}
			}
		}
	}

	return ACLSPV_COMPILE_OK;
}
