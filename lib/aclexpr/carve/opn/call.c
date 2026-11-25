#include "./call.h"
#include <aclemit/inline.h>
#include <aclctx/err.h>

#include <aclsym/fn.h>

#include <aclexpr/carve/util.h>

#include <aclcur/common.h>
#include <aclcur/opX.h>
#include <aclcur/reg.h>
#include <aclcur/load.h>
#include <aclcur/store.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <ae2f/pat/dbg.h>

/** has this register ever been the target of a store? */
static aclspv_bool_t s_ever_stored(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t	i;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t p = aclcur_get_node_idx(i, h_ctx);

		if(p && p->m_class == ACLCUR_STORE
				&& p->m_d.m_store.m_reg == c_reg)
			return 1;
	}

	return 0;
}

/** a call may have written state this function does not own: every register
 * whose storage the callee can reach gets a new value definition.
 *
 * that is exactly aclcur_reg_needs_memory(): a non-SCOPED register (global /
 * interface / pointer pointee) or an ALLOCATED one (its address escaped).
 * a plain Function-storage local lives in SSA and no callee can touch it.
 *
 * the new value is a memory load, and the register's cached value becomes
 * that load for the rest of the block:
 *
 *   untrack + CHANGED  -> the load path takes the memory seed, not the
 *                         pre-call SSA value
 *   load               -> one OpLoad, materialised by aclcur_emit only if
 *                         something refers to it
 *   implicit store     -> the register's value becomes that load
 *
 * the implicit store adds no node, so a read in another block does not
 * inherit the load; the label sweep untracks, and that read seeds from
 * memory itself — the same result, one load later.
 *
 * a register that was never stored has no stale SSA definition to replace:
 * untracking it is enough, its next read is a memory seed load anyway. */
static void s_clobber_reachable(h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t		i;
	const aclspv_wrd_t	c_n = h_ctx->m_num_cursor;

	ae2f_ifezerr(h_ctx->m_cursors.m_p) return;

	for(i = 0; i < c_n; ++i) {
		h_aclcur_t p = aclcur_get_node_idx(i, h_ctx);
		aclspv_id_t		c_reg;
		aclspv_id_t		c_val;

		if(!p || p->m_class != ACLCUR_REG)
			continue;

		c_reg = p->m_id;

		if(p->m_flag0 & (aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM)
					| aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS)))
			continue;

		/** the callee cannot reach SSA-only storage: leave its value alone.
		 * untracking one here would be worse than useless — the value exists
		 * only in the cache/SSA definition, and a later read in the same
		 * block would fall to a memory load of storage that was never
		 * written. */
		unless(aclcur_reg_needs_memory(c_reg, h_ctx))
			continue;

		if(!(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED))
				&& !s_ever_stored(c_reg, h_ctx)) {
			aclcur_reg_untrack(c_reg, h_ctx);
			continue;
		}

		/** creating nodes may move the cursor buffer: re-fetch by id. */
		aclcur_reg_untrack(c_reg, h_ctx);

		ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx)) continue;

		p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED);

		ae2f_ifezerr(c_val = aclcur_load_reg(c_reg, h_ctx, 0)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return;
		}

		/** the register's value for the rest of this block. no node is
		 * created, so nothing refers to the load here: a read does, and the
		 * emitter drops it otherwise. */
		ae2f_ifezerr(aclcur_store_reg_implicit(c_reg, c_val, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return;
		}
	}
}

aclspv_bool_t aclexpr_call_make(
		aclspv_id_t		c_fn,
		aclty_id_t		c_ty_ret,
		const aclspv_vid_t	c_args[],
		aclspv_wrd_t		c_num,
		aclspv_id_t* ae2f_restrict	wr_id,
		aclspv_bool_t* ae2f_restrict	wr_is_pure,
		h_aclspv_ctx_t		h_ctx
		) {
	aclsym_fn_t* ae2f_restrict p_sym;
	aclspv_bool_t			is_pure = 0;

	ae2f_ifezerr(*wr_id = aclcur_cfg_call(
				c_fn, c_ty_ret, c_args, c_num, h_ctx))
		return 1;

	p_sym = aclsym_fn_find_id(c_fn, h_ctx);

	/** the callee's promise is the call site's effect, and it propagates:
	 * a caller of a non-const callee may read external state, a caller of a
	 * non-pure callee may write it. an unknown callee promises nothing.
	 * CONST alone must not keep the clobbered caches alive (a const
	 * function may still write), but it does describe the reads. */
	{
		h_aclcur_t p_call = aclcur_get_node(*wr_id, h_ctx);

		if(p_call) {
			if(!p_sym || !(p_sym->m_flag0
						& aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST)))
				p_call->m_flag0 |= aclcur_flag0(
						ACLCUR_FLAG0_CALL_NONCONST);

			if(!p_sym || !(p_sym->m_flag0
						& aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE)))
				p_call->m_flag0 |= aclcur_flag0(
						ACLCUR_FLAG0_CALL_NONPURE);
		}
	}

	/** only PURE says "no writes". */
	if(p_sym)
		is_pure = (aclspv_bool_t)!!(p_sym->m_flag0
				& aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE));

	/** unknown callee counts as impure: conservatively reload. */
	unless(is_pure)
		s_clobber_reachable(h_ctx);

	if(wr_is_pure)
		*wr_is_pure = is_pure;

	return 0;
}

/** a pointer argument must reach OpFunctionCall as the memory object itself.
 * address-of wraps the storage id in OpCopyObject so the rest of the
 * expression machinery sees a typed value; the call must see the variable.
 *
 * a pointer into StorageBuffer / Uniform / ... needs SPIR-V variable
 * pointers; aclmit_fndecl_norm requests the capability when it emits a
 * function with such a parameter. */
static aclspv_id_t s_arg_value(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p = aclcur_get_node(c_id, h_ctx);

	if(!p || p->m_class != ACLCUR_OP1)
		return c_id;
	if(p->m_d.m_op.m_op != SpvOpCopyObject)
		return c_id;

	{
		aclty_t* ae2f_restrict p_ty = aclty_find(p->m_ty, h_ctx);

		if(p_ty && p_ty->m_class == ACLTY_PTR)
			return p->m_d.m_op.m_o[0];
	}

	return c_id;
}

aclspv_bool_t aclexpr_carve_opn_call(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		acltrl_t* ae2f_restrict			r_ltrl
		) {
	h_aclspv_ctx_t		ctx = CTX(rdwr_data);
	const aclspv_wrd_t	c_num = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_NUMOPRND];
	const aclspv_id_t	c_fn = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_SPECCONSTANTOP];
	const aclspv_id_t	c_ty_fn = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID];
	const aclty_id_t	c_ty_ret = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_vid_t		os[__ACLSPV_MAX_OPRND];
	aclspv_bool_t		is_pure = 0;
	aclty_t* ae2f_restrict	p_fn;
	aclspv_wrd_t		i;

	ae2f_ifezerr(p_fn = aclty_find(c_ty_fn, ctx))
		return 1;

	if(c_num + 1 > __ACLSPV_MAX_OPRND)
		return 1;

	for(i = 0; i < c_num; ++i) {
		const aclspv_id_t	c_id = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + i];
		const aclty_id_t	c_ty_from = rdwr_lst_extra[i].m_type_original;
		aclty_t* ae2f_restrict	p_arg;
		aclty_id_t		c_ty_to;

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_fn, i, ctx))
			return 1;

		c_ty_to = p_arg->m_info.m_fn_arg.m_prm_id;

		if(c_ty_from == c_ty_to) {
			os[i + 1] = s_arg_value(c_id, ctx);
		}
		else {
			acltrl_t		lit = rdwr_lst_extra[i].m_literal;
			enum ACLEXPR_RET_	R;

			R = aclexpr_arithmetic_cast(
					c_id
					, c_ty_from
					, rdwr_lst_extra[i].m_mask_literal
					, lit
					, &os[i + 1]
					, c_ty_to
					, 0
					, ctx
					);

			switch(R) {
				default:
				case ACLEXPR_RET_FAILURE:
				case ACLEXPR_RET_NOT_THE_CASE:
					err_puts("[carve::call] argument cast failed");
					return 1;

				case ACLEXPR_RET_SUCCESS:
				case ACLEXPR_RET_SUCCESS_CONSTANT:
				case ACLEXPR_RET_SUCCESS_LITERAL:
					break;
			}
		}
	}

	if(rdwr_lst_extra[c_num].m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_INLINE_CALL) {
		ae2f_ifnezerr(aclemit_inline_fn(
					rdwr_lst_extra[c_num].m_cursor
					, clang_getNullCursor()
					, &os[1]
					, c_num
					, rdwr_prv_idval
					, ctx))
			return 1;

		aclmemzero(r_ltrl, sizeof(*r_ltrl));

		rdwr_prv_extra->m_type_original = c_ty_ret;
		rdwr_prv_extra->m_mask_literal = 0;

		return 0;
	}

	ae2f_ifnezerr(aclexpr_call_make(
				c_fn, c_ty_ret, &os[1], c_num
				, rdwr_prv_idval, &is_pure, ctx))
		return 1;

	aclmemzero(r_ltrl, sizeof(*r_ltrl));

	rdwr_prv_extra->m_type_original	= c_ty_ret;
	rdwr_prv_extra->m_mask_literal	= ACLTRL_MASK_RT;
	rdwr_prv_extra->m_state0	= is_pure
		? (aclspv_wrd_t)0
		: (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_NOPURE;

	return 0;
}
