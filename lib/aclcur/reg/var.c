#include "./var.h"
#include "../common.h"
#include "../util.h"
#include "../ssa.h"
#include "../lbl/reach.h"

#include <aclty.h>
#include <aclspv.h>
#include <aclutil/emitx.h>
#include <ae2f/Cxx/Constexpr.h>

aclspv_id_t	aclcur_regvar_addr(aclspv_id_t c_reg, ae2f_unused h_aclspv_ctx_t h_ctx) {
	/** an OpVariable / OpFunctionParameter result id is already the
	 * pointer to the storage it declares, so the register id is its own
	 * address. taking one's address is an id passthrough: no opcode. */
	return c_reg;
}

aclconst_specsz	aclcur_regvar_size(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	ae2f_constexpr aclconst_specsz ZER = {0, };
	h_aclcur_t p;

	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return ZER;

	/** guard, mirroring resolve()/addr(): only a register node owns a
	 * type here. */
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
		return ZER;

	/** the register's storage is exactly one object of its own type. */
	return aclty_getsize(p->m_ty, h_ctx);
}

aclspv_compile_t	aclcur_regvar_emit_reg(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;
	aclspv_id_t ty;

	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;

	ae2f_ifezerr(ty = aclty_mk_ptr(
				p->m_ty
				, (enum SpvStorageClass_)p->m_d.m_reg.m_storage_class
				, h_ctx))
		return ACLSPV_COMPILE_ALLOC_FAILED_ID;

	if(p->m_d.m_reg.m_storage_class == SpvStorageClassFunction) {
		aclspv_wrd_t j;
		aclspv_bool_t keep;

		/** a function variable earns its OpVariable only when an emitted
		 * instruction references its storage: a seed or explicit load, an
		 * explicit (call-boundary) store, or the register being marked
		 * ACLCUR_FLAG0_REG_ALLOCATED — its address is taken, or an ALIAS2
		 * view chains onto it, so the storage must exist even with no
		 * live access. value-flow registers carry their values in SSA and
		 * never touch memory, so their variables would be dead weight —
		 * and a reference whose block is powered off at emission would
		 * dangle. */
		keep = (aclspv_bool_t)(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALLOCATED));
		unless(keep)
		for(j = 0; j < h_ctx->m_num_cursor; ++j) {
			h_aclcur_t q = &s_get_buf(h_ctx)[j];

			if(q->m_class == ACLCUR_LOAD
			&& q->m_d.m_load.m_reg == c_reg
			&& aclcur_ssa_lbl_live(q->m_lbl, h_ctx)) {
				keep = 1;
				break;
			}
			if(q->m_class == ACLCUR_STORE
			&& q->m_d.m_store.m_reg == c_reg
			&& aclcur_ssa_lbl_live(q->m_lbl, h_ctx)
			&& (q->m_flag0 & aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT))) {
				keep = 1;
				break;
			}
		}
		unless(keep)
			return ACLSPV_COMPILE_OK;

		ae2f_ifezerr(h_ctx->m_count.m_fndef = aclutil_emitx_4(
					&h_ctx->m_section.m_fndef
					, h_ctx->m_count.m_fndef
					, SpvOpVariable
					, ty
					, c_reg
					, p->m_d.m_reg.m_storage_class
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	} else {
		ae2f_ifezerr(h_ctx->m_count.m_vars = aclutil_emitx_4(
					&h_ctx->m_section.m_vars
					, h_ctx->m_count.m_vars
					, SpvOpVariable
					, ty
					, c_reg
					, p->m_d.m_reg.m_storage_class
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	}

	return ACLSPV_COMPILE_OK;
}

/** a merge with more predecessors than this falls back to a plain OpLoad. */
#define	ACLCUR_EMIT_PHI_MAX_PRED		64

/** register a predecessor label of the load's block, deduplicated.
 * 0 return: the predecessor buffer overflowed. */
static aclspv_bool_t	s_phi_pred_add(
		aclspv_id_t		c_pred,
		aclspv_id_t* ae2f_restrict	buf,
		aclspv_wrd_t		c_max,
		aclspv_wrd_t* ae2f_restrict	rdwr_n
		) {
	aclspv_wrd_t i;

	for(i = 0; i < *rdwr_n; ++i)
		if(buf[i] == c_pred)
			return 1;

	if(*rdwr_n >= c_max)
		return 0;

	buf[(*rdwr_n)++] = c_pred;
	return 1;
}

/** label id of the block the cursor at idx sits in.
 * the root label occupies cursor 0, so the backward scan terminates. */
static aclspv_id_t	s_phi_block_lbl(aclspv_wrd_t idx, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t j = idx;

	while(j) {
		h_aclcur_t q = &s_get_buf(h_ctx)[--j];
		if(q->m_class == ACLCUR_LBL)
			return q->m_id;
	}

	return 0;
}

/** decide whether p_load may be emitted as OpPhi instead of OpLoad.
 *
 * passes iff: the load's block has at least one immediate predecessor,
 * every immediate predecessor carries a phi entry (entries whose label is
 * not a predecessor are DFS artifacts and are dropped), each entry's store
 * resolves to a real value, and nothing that emits into the function body
 * precedes the load inside its block (OpPhi must be the first instruction).
 *
 * on success, rdwr_keeps[] holds (value, label) pairs and *rdwr_n the count.
 * */
static aclspv_bool_t	s_phi_emit_ready(
		h_aclcur_t			p_load,
		h_aclspv_ctx_t			h_ctx,
		aclspv_id_t* ae2f_restrict	rdwr_keeps,
		aclspv_wrd_t* ae2f_restrict	rdwr_n
		) {
	aclspv_wrd_t i;
	aclspv_wrd_t npd;
	aclspv_id_t pds[ACLCUR_EMIT_PHI_MAX_PRED];
	aclspv_id_t c_lbl;
	aclspv_id_t id_phi;
	h_aclcur_t p_lbl;

	c_lbl	= p_load->m_lbl;
	npd	= 0;

	/** a loop block's memory load stays a plain OpLoad: the phi's
	 * back-edge operand would be defined in a block that does not
	 * dominate the merge. */
	if(aclcur_lbl_in_cycle(c_lbl, h_ctx))
		return 0;

	/** immediate predecessors of the load's block: every branch whose
	 * target is the block contributes the block it sits in. */
	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t q = &s_get_buf(h_ctx)[i];

		switch(q->m_class) {
			case ACLCUR_J:
				if(q->m_d.m_j.m_lbl != c_lbl)
					break;
				if(!s_phi_pred_add(s_phi_block_lbl(i, h_ctx), pds, ACLCUR_EMIT_PHI_MAX_PRED, &npd))
					return 0;
				break;

			case ACLCUR_JC:
				if(q->m_d.m_jc.m_lbl_true == c_lbl
				|| q->m_d.m_jc.m_lbl_false == c_lbl
				|| q->m_d.m_jc.m_merge == c_lbl) {
					if(!s_phi_pred_add(s_phi_block_lbl(i, h_ctx), pds, ACLCUR_EMIT_PHI_MAX_PRED, &npd))
						return 0;
				}
				break;

			default:
				break;
		}
	}

	/** no predecessor at all: a phi needs at least one (value, label) pair. */
	if(!npd)
		return 0;

	/** exactness: every predecessor must carry a phi entry. */
	*rdwr_n	= 0;
	id_phi	= p_load->m_d.m_load.m_phi;
	while(id_phi) {
		h_aclcur_t q_phi	= aclcur_get_node(id_phi, h_ctx);
		aclspv_wrd_t j;

		if(!q_phi)
			return 0;

		for(j = 0; j < npd; ++j) {
			if(q_phi->m_d.m_phi.m_lbl == pds[j]) {
				h_aclcur_t p_store = aclcur_get_node(q_phi->m_val, h_ctx);

				/** the phi's value: the STORE node's stored value. */
				if(!p_store || !p_store->m_val)
					return 0;

				rdwr_keeps[(*rdwr_n) * 2]	= p_store->m_val;
				rdwr_keeps[(*rdwr_n) * 2 + 1]	= q_phi->m_d.m_phi.m_lbl;
				++*rdwr_n;
				break;
			}
		}

		id_phi	= q_phi->m_d.m_phi.m_nxt;
	}

	if(*rdwr_n != npd)
		return 0;

	/** OpPhi must be the first instruction of its block: anything emitting
	 * into the function body before the load forces a plain OpLoad. */
	p_lbl	= aclcur_get_node(c_lbl, h_ctx);
	if(!p_lbl || p_lbl->m_class != ACLCUR_LBL)
		return 0;

	{
		const aclspv_wrd_t idx		= (aclspv_wrd_t)(p_load - s_get_buf(h_ctx));
		const aclspv_wrd_t lbl_idx	= (aclspv_wrd_t)(p_lbl - s_get_buf(h_ctx));

		for(i = lbl_idx + 1; i < idx; ++i) {
			h_aclcur_t q = &s_get_buf(h_ctx)[i];
			switch(q->m_class) {
				case ACLCUR_J:
				case ACLCUR_JC:
				case ACLCUR_RET:
				case ACLCUR_LOAD:
				case ACLCUR_STORE:
				case ACLCUR_OP1:
				case ACLCUR_OP2:
				case ACLCUR_OP3:
				case ACLCUR_OPX:
					return 0;
				default:
					break;
			}
		}
	}

	return 1;
}

/** identity pointer producer: a stock variable register's memory operand
 * is its own id — an OpVariable / OpFunctionParameter result id is
 * already the pointer to its storage. */
static aclspv_compile_t	s_id_ptr(
		aclspv_id_t c_reg
		, aclspv_id_t* ae2f_restrict wr_ptr
		, ae2f_unused h_aclspv_ctx_t h_ctx
		) {
	*wr_ptr	= c_reg;
	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t	aclcur_regvar_emit_loadof(
		aclspv_id_t c_id
		, h_aclspv_ctx_t h_ctx
		, aclcur_regvar_ptr_t* fn_ptr
		, aclspv_bool_t c_volatile
		) {
	h_aclcur_t p;
	h_aclcur_t p_reg;
	aclspv_compile_t err;
	aclspv_id_t id_ptr;
	aclspv_wrd_t nkeep;
	aclspv_wrd_t i;
	aclspv_id_t keeps[ACLCUR_EMIT_PHI_MAX_PRED * 2];

	ae2f_ifezerr(p = aclcur_get_node(c_id, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifezerr(p_reg = aclcur_get_node(p->m_d.m_load.m_reg, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;

	/** short-circuit merge: a complete phi chain on the load means the value
	 * converges here; emit OpPhi in place of the OpLoad. the phi
	 * materialization is pointer-independent — only the OpLoad path
	 * consults the producer — and the explicit (volatile) form takes the
	 * load path directly. */
	if(!c_volatile
	&& p->m_d.m_load.m_num_phi
	&& s_phi_emit_ready(p, h_ctx, keeps, &nkeep)) {
		const aclspv_wrd_t	ANCHOUR = h_ctx->m_count.m_fnimpl;
		aclspv_wrd_t* ae2f_restrict wr;

		/** OpPhi layout: [opcode|3+2n] [ty] [result] (value label)*n */
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emit_wrd(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl + 2 + 2 * nkeep
					, 0
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

		wr	= &aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fnimpl)[ANCHOUR];
		wr[0]	= aclspv_opcode_to_wrd(SpvOpPhi, 2 + 2 * nkeep);
		wr[1]	= p_reg->m_ty;
		wr[2]	= c_id;

		for(i = 0; i < nkeep; ++i) {
			wr[3 + 2 * i]		= keeps[2 * i];
			wr[3 + 2 * i + 1]	= keeps[2 * i + 1];
		}

		return ACLSPV_COMPILE_OK;
	}

	/** the OpLoad memory operand: the register id by default, or the
	 * producer's pointer — e.g. an ALIAS2 view chain minted as a
	 * freshly emitted OpAccessChain. */
	ae2f_ifnezerr(err = fn_ptr(p->m_d.m_load.m_reg, &id_ptr, h_ctx))
		return err;

	if(c_volatile) {
		/** volatile load: OpLoad with the Volatile memory-access operand. */
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_5(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpLoad
					, p_reg->m_ty
					, c_id
					, id_ptr
					, SpvMemoryAccessVolatileMask
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	} else {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_4(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpLoad
					, p_reg->m_ty
					, c_id
					, id_ptr
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	}

	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t	aclcur_regvar_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	/** stock load: the register id is its own memory operand. */
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_id_ptr, 0);
}

aclspv_compile_t	aclcur_regvar_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	/** stock explicit (volatile) load: the register id is its own memory
	 * operand. */
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_id_ptr, 1);
}

aclspv_compile_t	aclcur_regvar_emit_storeof(
		aclspv_id_t c_id
		, h_aclspv_ctx_t h_ctx
		, aclcur_regvar_ptr_t* fn_ptr
		, aclspv_bool_t c_volatile
		) {
	h_aclcur_t p;
	aclspv_compile_t err;
	aclspv_id_t id_ptr;

	ae2f_ifezerr(p = aclcur_get_node(c_id, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;

	/** non-explicit stores are pure SSA definitions of the register's
	 * value: the value flows through the block-local tracking and the
	 * phi graph; storage is written only at explicit (call-boundary)
	 * stores. */
	unless(c_volatile
	|| (p->m_flag0 & aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT)))
		return ACLSPV_COMPILE_OK;

	/** the OpStore memory operand: the register id by default, or the
	 * producer's pointer. */
	ae2f_ifnezerr(err = fn_ptr(p->m_d.m_store.m_reg, &id_ptr, h_ctx))
		return err;

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_3(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, SpvOpStore
				, id_ptr
				, p->m_val
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t	aclcur_regvar_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	/** stock store: the register id is its own memory operand; the
	 * ACLCUR_FLAG0_STORE_EXPLICIT gating lives in the of-variant. */
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_id_ptr, 0);
}

aclspv_compile_t	aclcur_regvar_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	/** stock explicit store: the register id is its own memory operand. */
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_id_ptr, 1);
}
