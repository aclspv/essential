#include "./view.h"
#include "../common.h"
#include "../op.h"

#include <aclty.h>
#include <aclspv.h>
#include <aclutil/emitx.h>
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
	aclspv_wrd_t nidx;
	aclspv_wrd_t bound;
	aclspv_id_t id_cur;
	h_aclcur_t p;
	h_aclcur_t p_self;
	aclspv_id_t id_base;
	aclspv_wrd_t i;

	/** the view register whose address is requested. */
	ae2f_ifezerr(p_self = aclcur_get_node(c_reg, h_ctx))
		return 0;
	ae2f_ifnezerr_strict(p_self->m_class != ACLCUR_REG)
		return 0;

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

	id_base	= p->m_id;

	for(i = 0; i < nidx; ++i) {
		/** idxs[]/tys[] were filled view-first: take them root-most
		 * first, pairing each index with its own landing type. */
		const aclspv_wrd_t J = nidx - 1 - i;
		const aclspv_id_t TY_ELEM = aclty_mk_ptr(
				tys[J]
				, (enum SpvStorageClass_)p_self->m_d.m_reg.m_storage_class
				, h_ctx
				);
		aclspv_id_t NEXT;

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

aclspv_id_t aclcur_reg_view(
		CXCursor		c_cur,
		aclspv_id_t		c_parent,
		aclspv_id_t		c_ty,
		aclspv_id_t		c_idx,
		aclspv_wrd_t		c_storage_class,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t id;
	h_aclcur_t p;

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

	/** ALIAS2 link: also propagates the kept-flag (ACLCUR_FLAG0_REG_
	 * ALLOCATED) onto the parent — transitively the whole chain. */
	ae2f_ifezerr(aclcur_reg_alias2(id, c_parent, h_ctx))
		return 0;

	return id;
}
