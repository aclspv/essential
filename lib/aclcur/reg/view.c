#include "./view.h"
#include "./var.h"
#include "../common.h"
#include "../op.h"
#include "../util.h"

#include <util/log.h>
#include <aclty.h>
#include <aclspv.h>
#include <aclutil/emitx.h>
#include <aclctx/id.h>
#include <ae2f/Cxx/Constexpr.h>

/** OpAccessChain index-chain depth cap (fixed-size accumulator). */
#define	ACLCUR_REGVIEW_CHAIN_MAX		__ACLSPV_OPACC_IDX_MAX

/** the address of an ALIAS2 view: one OpAccessChain per chain level,
 * root-most first, each a real node.
 *
 * node-based, not words appended into m_fnimpl: the index of a dynamic
 * subscript (`a[idx]`) is itself an instruction emitted during the emit pass,
 * so a chain appended during evaluation would reference an id that does not
 * exist yet. Configuring the chain as a node lets the emit pass order it
 * after whatever its operands came from.
 *
 * applied level by level rather than as one multi-index instruction because
 * aclcur_cfg_op1i takes exactly one index; successive single-index chains are
 * equivalent. OpAccessChain is pure — it reads no memory — so its result
 * depends only on its operands, which is what makes the dedup inside those
 * helpers sound here. (The same dedup on a load would NOT be sound.)
 *
 * still uncached, and called once per use: each call is a distinct
 * materialisation, so no caller may hold on to the result across a
 * re-materialisation.
 *
 * @return 0 on failure.
 * */
aclspv_id_t	aclcur_regview_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	aclspv_id_t idxs[ACLCUR_REGVIEW_CHAIN_MAX];
	aclspv_id_t tys[ACLCUR_REGVIEW_CHAIN_MAX];
	aclspv_wrd_t steps[ACLCUR_REGVIEW_CHAIN_MAX];
	aclspv_wrd_t nidx;
	aclspv_wrd_t bound;
	aclspv_id_t id_cur;
	h_aclcur_t p;
	h_aclcur_t p_self;
	aclspv_id_t id_base;
	aclspv_wrd_t i;
	aclspv_wrd_t storage;

	/** the view register whose address is requested. */
	ae2f_ifezerr(p_self = aclcur_get_node(c_reg, h_ctx))
		return 0;
	ae2f_ifnezerr_strict(p_self->m_class != ACLCUR_REG)
		return 0;

	/** capture the storage class now: the chain mint below grows the cursor
	 * arena, which would leave p_self dangling. every level shares the
	 * root's storage class. */
	storage	= p_self->m_d.m_reg.m_storage_class;

	/** ancestor walk: from this view up the ALIAS2 parent links,
	 * collecting each m_id_idx together with the type that step lands on —
	 * this view's own first, root-most last (reversed below). iterative,
	 * never recursive: call-stack safety. bounded by the cursor count (a
	 * cyclic parent chain would never terminate otherwise) and by
	 * ACLCUR_REGVIEW_CHAIN_MAX (the fixed accumulator). */
	nidx	= 0;
	bound	= h_ctx ? h_ctx->m_num_cursor : 0;
	id_cur	= c_reg;
	p	= 0;
	while(bound-- && nidx < ACLCUR_REGVIEW_CHAIN_MAX) {
		ae2f_ifezerr(p = aclcur_get_node(id_cur, h_ctx))
			return 0;
		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
			return 0;

		/** the root register (the var / parameter owning the storage) has
		 * no ALIAS2 flag: its m_addr is the pure-identity passthrough. */
		unless(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
			break;

		idxs[nidx]	= p->m_d.m_reg.m_id_idx;
		/** the type this step lands on is this level's own type. */
		tys[nidx]	= p->m_ty;
		/** the kind of step this level adds (MEMBER / ELEMENT / DEREF). */
		steps[nidx]	= p->m_d.m_reg.m_step;
		++nidx;
		id_cur		= p->m_d.m_reg.m_d.m_ref.m_id;
	}

	/** the walk must have stopped on a root: exhausting the bound (cyclic
	 * parent chain) or the accumulator (chain deeper than
	 * ACLCUR_REGVIEW_CHAIN_MAX) is a failure. */
	ae2f_ifezerr(p)
		return 0;
	ae2f_ifnezerr_strict(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		return 0;

	/** the chain roots on the root register's ANCHOR, not its own id: m_base
	 * for a ghost (a held pointer value), else the register's own id (the
	 * real-register identity sentinel). see aclcur_reg_base(). */
	ae2f_ifezerr(id_base = aclcur_reg_base(p->m_id, h_ctx))
		return 0;

	/** materializing this view's address references the root's storage, so
	 * ask for it explicitly: a Function-storage `&a[i]` emits no live load
	 * or explicit store through the view, and without this the root earns
	 * no OpVariable and the OpAccessChain below names an undefined base.
	 * this replaces the eager ALLOCATED propagation aclcur_reg_alias2 used
	 * to do — the request is now made exactly where the storage is
	 * actually needed. the node pointer above is re-fetched because the
	 * chain mint below allocates. */
	{
		h_aclcur_t p_root = aclcur_get_node(p->m_id, h_ctx);

		if(p_root)
			p_root->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALLOCATED);
	}

	for(i = 0; i < nidx; ++i) {
		/** idxs[]/tys[] were filled view-first: take them root-most
		 * first, pairing each index with its own landing type. */
		const aclspv_wrd_t J = nidx - 1 - i;
		const aclspv_id_t TY_ELEM = aclty_mk_ptr(
				tys[J]
				, (enum SpvStorageClass_)storage
				, h_ctx
				);
		aclspv_id_t NEXT;

		/** P0: only indexed steps (MEMBER / ELEMENT) are materialized, both
		 * as an OpAccessChain append. a DEREF step rebases the running
		 * address to the parent's pointer value and lands with the ghost
		 * work (P3). */
		ae2f_ifnezerr_strict(steps[J] != ACLCUR_REGVIEW_STEP_MEMBER
				&& steps[J] != ACLCUR_REGVIEW_STEP_ELEMENT)
			return 0;

		ae2f_ifezerr(TY_ELEM)
			return 0;

		/** the index must be kept alive by THIS chain.
		 *
		 * it rides in the op1i/op1ri immediate slot (m_o[1]), not in the
		 * operand slot, so those helpers reference only c_o0. for a
		 * dynamic subscript the index is an instruction of its own — the
		 * OpISub of `a[i - 1]` — and an unreferenced ACLCUR_OP2 is
		 * dropped by the emit pass (see EMIT_aclcur_op2's m_num_ref gate).
		 * the chain then names an id nothing defines.
		 *
		 * taken before the dedup inside those helpers, deliberately: on the
		 * dedup path an equivalent chain already exists and still needs its
		 * index alive. over-counting by one is harmless — m_num_ref only
		 * gates elision, and never being decremented here matches every
		 * other operand reference in the layer. */
		{
			const h_aclcur_t p_idx = idxs[J]
				? aclcur_get_node(idxs[J], h_ctx) : 0;

			if(p_idx)	(void)++p_idx->m_num_ref;
		}

		/** the first level's base is the root register's own id, which is
		 * not a value node (a register carries no VALUE flag); every later
		 * level's base is the previous chain's result, which is. */
		if(0 == i)
			NEXT = aclcur_cfg_op1ri(
						0
						, SpvOpAccessChain
						, TY_ELEM
						, id_base
						, idxs[J]
						, h_ctx
						);
		else
			NEXT = aclcur_cfg_op1i(
						0
						, SpvOpAccessChain
						, TY_ELEM
						, id_base
						, idxs[J]
						, h_ctx
						);

		ae2f_ifezerr(NEXT)
			return 0;

		id_base = NEXT;
	}

	return id_base;
}

aclconst_specsz	aclcur_regview_size(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	ae2f_constexpr aclconst_specsz ZER = {0, };
	h_aclcur_t p;

	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return ZER;

	/** guard, mirroring resolve()/addr(): only a register node owns a type. */
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
		return ZER;

	/** the view's storage is exactly one object of its own type. */
	return aclty_getsize(p->m_ty, h_ctx);
}

/** emit-time view chain: the OpAccessChain a view's load/store memory
 * operand needs, emitted as RAW WORDS into the current function body,
 * root-most level first.
 *
 * raw words, not aclcur_regview_addr's op-node: minting a node at emit
 * time appends it at (or reuses a slot behind) the cursor the emit pass
 * is currently on, so the OpAccessChain would either be emitted after the
 * OpLoad / OpStore that names it or not at all. appending the words
 * immediately before lands it in the right order. this is the same shape
 * the push-constant leaf hooks use (aclemit/fndecl/entp/args.c).
 *
 * every index is already emitted by the time a LOAD / STORE node runs:
 * the node that defines a dynamic subscript sits earlier in the cursor
 * order, and a view's index is kept referenced from creation
 * (aclcur_reg_view). so the chain never names an undefined id.
 *
 * @return ACLSPV_COMPILE_OK, or an error code.
 * */
static aclspv_compile_t	s_regview_chain(
		aclspv_id_t c_reg
		, aclspv_id_t* ae2f_restrict wr_ptr
		, h_aclspv_ctx_t h_ctx
		) {
	aclspv_id_t idxs[ACLCUR_REGVIEW_CHAIN_MAX];
	aclspv_id_t tys[ACLCUR_REGVIEW_CHAIN_MAX];
	aclspv_wrd_t steps[ACLCUR_REGVIEW_CHAIN_MAX];
	aclspv_wrd_t nidx;
	aclspv_wrd_t bound;
	aclspv_id_t id_cur;
	h_aclcur_t p;
	h_aclcur_t p_self;
	aclspv_id_t id_base;
	aclspv_wrd_t i;

	/** the view register whose chain is requested. */
	ae2f_ifezerr(p_self = aclcur_get_node(c_reg, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p_self->m_class != ACLCUR_REG)
		return ACLSPV_COMPILE_UNHANDLED;

	/** ancestor walk, exactly as aclcur_regview_addr's: view-first. */
	nidx	= 0;
	bound	= h_ctx ? h_ctx->m_num_cursor : 0;
	id_cur	= c_reg;
	p	= 0;
	while(bound-- && nidx < ACLCUR_REGVIEW_CHAIN_MAX) {
		ae2f_ifezerr(p = aclcur_get_node(id_cur, h_ctx))
			return ACLSPV_COMPILE_UNHANDLED;
		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
			return ACLSPV_COMPILE_UNHANDLED;

		unless(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
			break;

		idxs[nidx]	= p->m_d.m_reg.m_id_idx;
		tys[nidx]	= p->m_ty;
		steps[nidx]	= p->m_d.m_reg.m_step;
		++nidx;
		id_cur		= p->m_d.m_reg.m_d.m_ref.m_id;
	}

	ae2f_ifezerr(p)
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		return ACLSPV_COMPILE_UNHANDLED;

	/** the chain roots on the root register's ANCHOR: m_base for a ghost,
	 * else the register's own id. see aclcur_reg_base(). */
	ae2f_ifezerr(id_base = aclcur_reg_base(p->m_id, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;

	for(i = 0; i < nidx; ++i) {
		const aclspv_wrd_t J = nidx - 1 - i;
		const aclspv_id_t TY_ELEM = aclty_mk_ptr(
				tys[J]
				, (enum SpvStorageClass_)p_self->m_d.m_reg.m_storage_class
				, h_ctx
				);
		aclspv_id_t id_nxt;

		/** only indexed steps are an OpAccessChain append; a DEREF step
		 * rebases to the parent's pointer value and lands with the ghost
		 * work (P3). */
		ae2f_ifnezerr_strict(steps[J] != ACLCUR_REGVIEW_STEP_MEMBER
				&& steps[J] != ACLCUR_REGVIEW_STEP_ELEMENT)
			return ACLSPV_COMPILE_UNHANDLED;

		ae2f_ifezerr(TY_ELEM)
			return ACLSPV_COMPILE_ALLOC_FAILED_ID;

		ae2f_ifezerr(id_nxt = aclctx_newid(h_ctx))
			return ACLSPV_COMPILE_ALLOC_FAILED_ID;

		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_5(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpAccessChain
					, TY_ELEM
					, id_nxt
					, id_base
					, idxs[J]
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

		id_base	= id_nxt;
	}

	*wr_ptr	= id_base;
	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t	aclcur_regview_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_regview_chain, 0);
}

aclspv_compile_t	aclcur_regview_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_regview_chain, 1);
}

aclspv_compile_t	aclcur_regview_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_regview_chain, 0);
}

aclspv_compile_t	aclcur_regview_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_regview_chain, 1);
}

aclspv_id_t aclcur_reg_view(
		CXCursor		c_cur,
		aclspv_id_t		c_parent,
		aclspv_id_t		c_ty,
		aclspv_id_t		c_idx,
		aclspv_wrd_t		c_storage_class,
		aclspv_wrd_t		c_step,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t id;
	h_aclcur_t p;
	h_aclcur_t p_parent;
	aclspv_id_t idc;

	ae2f_ifezerr(p_parent = aclcur_get_node(c_parent, h_ctx))
		return 0;
	ae2f_ifnezerr_strict(p_parent->m_class != ACLCUR_REG)
		return 0;

	/** canonical dedup (D1): a register owns exactly one ALIAS2 parent, so
	 * the view of a given (parent, step, idx, ty) region is unique. the
	 * existing sibling is reused rather than duplicated — a correctness
	 * precondition for views as l-values (two nodes for one region would
	 * each carry their own m_val and diverge). the child list is walked by
	 * id and every node is re-fetched: aclcur_get_node does not allocate,
	 * so nothing moves under us mid-scan. */
	for(idc = p_parent->m_d.m_reg.m_id_child_head; idc; ) {
		h_aclcur_t pc = aclcur_get_node(idc, h_ctx);
		aclspv_id_t id_nxt;

		if(!pc || pc->m_class != ACLCUR_REG)
			break;

		/** read before the next re-fetch, so the walk survives any move. */
		id_nxt = pc->m_d.m_reg.m_id_child_nxt;

		if((pc->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		&& pc->m_d.m_reg.m_step == c_step
		&& pc->m_d.m_reg.m_id_idx == c_idx
		&& pc->m_ty == c_ty) {
			dbg_prefix();
			dbg_call(fprintf, (stderr
						, "[dedup] view reused: parent %u idx %u ty %u -> %u\n"
						, (unsigned)c_parent, (unsigned)c_idx
						, (unsigned)c_ty, (unsigned)idc));
			return idc;
		}

		idc = id_nxt;
	}

	/** the view is a compile-time node: it owns no storage and emits no
	 * instruction of its own, so its cursor is allocated in the skipped
	 * (c_toskip=1) path — it must not corrupt block extents. */
	ae2f_ifezerr(id = aclcur_cfg_node2(0, h_ctx, 1))
		return 0;
	ae2f_ifezerr(p = aclcur_get_node(id, h_ctx))
		return 0;

	p->m_class			= ACLCUR_REG;
	p->m_cur			= c_cur;
	p->m_ty				= c_ty;
	p->m_d.m_reg.m_storage_class	= c_storage_class;
	p->m_d.m_reg.m_in		= aclcur_regview_in().m_in;
	p->m_d.m_reg.m_id_idx		= c_idx;
	/** the step kind is explicit (D2): the chain materialization and the
	 * dedup identity both key on it, never on the parent's m_ty. */
	p->m_d.m_reg.m_step		= c_step;
	/** a view's own base is not consulted (the chain walk roots on the
	 * root register's base); 0 keeps the identity sentinel. */
	p->m_d.m_reg.m_base		= 0;

	/** NO SCOPED snapshot: a view's memory policy is resolved from its
	 * storage root on demand (aclcur_reg_needs_memory), never copied here.
	 * a copy would freeze the mode at creation — an address taken later
	 * would be missed, and the view would wrongly stay SSA. */

	/** the view's chain names m_id_idx: keep the index's definition alive
	 * from here on. the emit pass elides an unreferenced op, and the chain
	 * is only emitted later, at the view's load/store. a module constant
	 * has no cursor node and needs nothing. */
	{
		h_aclcur_t p_idx = c_idx ? aclcur_get_node(c_idx, h_ctx) : 0;

		if(p_idx)	(void)++p_idx->m_num_ref;
	}

	/** ALIAS2 link: parent-relative identity only; materialization is
	 * liveness-driven (see aclcur_reg_alias2). */
	ae2f_ifezerr(aclcur_reg_alias2(id, c_parent, h_ctx))
		return 0;

	return id;
}

/* ================================================================== */
/* Ghost registers (D6): a chain root anchored on a held pointer VALUE. */
/*                                                                    */
/* a ghost is a parentless DEREF view: it owns no storage and its m_base  */
/* is the pointer value a dereference yielded. its m_id is a fresh        */
/* compile-time handle, distinct from m_base, so aclcur_reg_base() can    */
/* tell the two apart.                                                    */
/* ================================================================== */

aclspv_id_t	aclcur_regghost_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	/** identity: `&*p` is `p`. the held pointer value lives in m_base, and
	 * this is a pure, idempotent passthrough (unlike a view's m_addr). */
	return aclcur_reg_base(c_reg, h_ctx);
}

/** memory operand producer for a ghost's load/store: the held pointer value. */
static aclspv_compile_t	s_regghost_ptr(
		aclspv_id_t			c_reg,
		aclspv_id_t* ae2f_restrict	wr_ptr,
		h_aclspv_ctx_t			h_ctx
		) {
	const aclspv_id_t base = aclcur_reg_base(c_reg, h_ctx);

	ae2f_ifezerr(base)
		return ACLSPV_COMPILE_UNHANDLED;

	*wr_ptr = base;
	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t	aclcur_regghost_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_regghost_ptr, 0);
}

aclspv_compile_t	aclcur_regghost_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_regghost_ptr, 1);
}

aclspv_compile_t	aclcur_regghost_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_regghost_ptr, 0);
}

aclspv_compile_t	aclcur_regghost_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_regghost_ptr, 1);
}

aclspv_id_t aclcur_reg_ghost(aclspv_id_t c_base, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_base;
	const aclty_t* p_ty;
	aclspv_id_t c_poi;
	aclspv_wrd_t c_storage;
	aclspv_id_t id;
	h_aclcur_t p;
	aclspv_wrd_t i;

	ae2f_ifezerr(p_base = aclcur_get_node(c_base, h_ctx))
		return 0;

	/** the held value must be a pointer: its pointee is the ghost's own
	 * region type, its storage class the ghost's. */
	ae2f_ifezerr(p_ty = aclty_find(p_base->m_ty, h_ctx))
		return 0;

	ae2f_ifnezerr_strict(p_ty->m_class != ACLTY_PTR)
		return 0;

	c_poi		= p_ty->m_info.m_ptr.m_type;
	c_storage	= (aclspv_wrd_t)p_ty->m_info.m_ptr.m_storage;

	/** canonical dedup: one ghost per (held pointer value, pointee). a
	 * pointer value plus a pointee names one region, so two member accesses
	 * through the same pointer share one ghost and, with aclcur_reg_view's
	 * dedup, one MEMBER view. the scan is linear and does not allocate: a
	 * node handle stays valid across it. */
	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t pc = &s_get_buf(h_ctx)[i];

		if(!pc || !pc->m_id || pc->m_class != ACLCUR_REG)
			continue;
		if(!(pc->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_GHOST)))
			continue;
		if(pc->m_d.m_reg.m_base == c_base && pc->m_ty == c_poi)
			return pc->m_id;
	}

	ae2f_ifezerr(id = aclcur_cfg_node2(0, h_ctx, 1))
		return 0;
	ae2f_ifezerr(p = aclcur_get_node(id, h_ctx))
		return 0;

	p->m_class			= ACLCUR_REG;
	p->m_ty				= c_poi;
	p->m_d.m_reg.m_storage_class	= c_storage;
	p->m_d.m_reg.m_in		= aclcur_regghost_in().m_in;
	p->m_d.m_reg.m_base		= c_base;
	p->m_d.m_reg.m_step		= ACLCUR_REGVIEW_STEP_DEREF;
	p->m_flag0			|= aclcur_flag0_reg(ACLCUR_FLAG0_REG_GHOST);

	/** aclcur_cfg_node2 above may have grown the cursor buffer and moved it,
	 * so the node handle taken at entry is stale: re-fetch it before use.
	 * (a handle is only stable while no cursor-allocating call runs.) */
	ae2f_ifezerr(p_base = aclcur_get_node(c_base, h_ctx))
		return 0;

	/** the ghost's chain (and any view over it) names the held value later;
	 * keep its definition alive from here on — the emit pass elides an
	 * unreferenced op, and the chain is only emitted at the view's
	 * load/store. */
	(void)++p_base->m_num_ref;

	return id;
}

aclspv_id_t	aclcur_reg_ptrval(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;

	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)		return 0;
	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))	return 0;
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)	return 0;

	/** D7: the register KIND decides, never `m_ty == ACLTY_PTR`. */
	if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		return p->m_d.m_reg.m_in.m_addr
				? p->m_d.m_reg.m_in.m_addr(c_reg, h_ctx)
				: 0;

	if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_GHOST))
		return aclcur_reg_base(c_reg, h_ctx);

	return aclcur_load_reg(c_reg, h_ctx, 0);
}
