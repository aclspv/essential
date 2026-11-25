#ifndef	aclexpr_visit_opn_call_h
#define	aclexpr_visit_opn_call_h

#include <aclutil/scale.h>
#include <aclctx/err.h>

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/carve/opn/call.h>

#include <aclsym/fn.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <util/visitfromroot.h>

#include <aclmem.h>
#include <aclutil/fn.h>
#include <aclattr/fnctrl.h>
#include <aclemit/inline.h>
#include <aclexpr/ctor.h>

#include <aclcur/reg.h>
#include <aclexpr/carve/ptr.h>
#include <aclexpr/util/unwrap.h>

/**
 * a call to a user function (`f(a, b)`).
 *
 * the callee must be a cached, already emitted symbol: a body-less callee is
 * an import, and imports land with the build phase. anything not in the cache
 * is a builtin / header function and is left to the lanes that own those
 * (glslmath, shuffle, ...), or to the final reject.
 *
 * arguments are visited through the normal expression machinery so each one
 * is carved with its own type; the call carve converts them to the parameter
 * types. the scale carries the callee (SPECCONSTANTOP), its function type
 * (RESID) and the result type (RESTYID) — the callee is not an operand value.
 *
 * a zero-argument call has nothing to fill a scale with, so it is built here
 * and handed to the ascend label as a value.
 * */
/** the object pointer a member call passes as its implicit first argument.
 *
 * `a.get()` takes the address of the local, `p->get()` passes p's value, and
 * an implicit `get()` inside another method passes the current `this`. */

/** should this callee be inlined at the call site? */
static aclspv_bool_t s_should_inline(
		CXCursor	c_callee,
		h_aclspv_ctx_t	h_ctx
		) {
	/** call-site inlining is staged: the inliner and the
	 * INLINE_BY_DEFAULT flag are in place, but ordinary calls still emit
	 * OpFunctionCall until the inliner is stable. subobject destructors
	 * use the inliner directly. */
	(void)c_callee;
	(void)h_ctx;
	return 0;
}

static aclspv_bool_t s_method_object(
		CXCursor		c_call,
		h_aclspv_ctx_t		h_ctx,
		aclspv_id_t* ae2f_restrict	wr_id,
		aclty_id_t* ae2f_restrict	wr_ty
		) {
	aclexpr_opnd_t	CALLEE, OBJ;
	CXCursor	BASE;
	aclspv_id_t	REG;
	h_aclcur_t	p_reg;
	const aclty_t* ae2f_restrict p_ty;

	CALLEE = aclexpr_opnd_first_child(c_call);

	unless(CALLEE.m_has)
		return 0;

	OBJ = aclexpr_opnd_first_child(CALLEE.m_c);

	if(!OBJ.m_has) {
		if(!h_ctx->m_this)
			return 0;

		ae2f_ifezerr(p_reg = aclcur_get_node(h_ctx->m_this, h_ctx))
			return 0;

		ae2f_ifezerr(*wr_id = aclexpr_ptrval(h_ctx->m_this, h_ctx))
			return 0;

		*wr_ty = p_reg->m_ty;
		return 1;
	}

	BASE = aclexpr_expr_unwrap(OBJ.m_c);

	if(clang_getCursorKind(BASE) == CXCursor_CXXThisExpr) {
		if(!h_ctx->m_this)
			return 0;

		ae2f_ifezerr(p_reg = aclcur_get_node(h_ctx->m_this, h_ctx))
			return 0;

		ae2f_ifezerr(*wr_id = aclexpr_ptrval(h_ctx->m_this, h_ctx))
			return 0;

		*wr_ty = p_reg->m_ty;
		return 1;
	}

	if(clang_getCursorKind(BASE) == CXCursor_UnaryOperator
			&& clang_getCursorUnaryOperatorKind(BASE) == CXUnaryOperator_Deref) {
		aclexpr_opnd_t U = aclexpr_opnd_first_child(BASE);

		unless(U.m_has)
			return 0;

		BASE = aclexpr_expr_unwrap(U.m_c);
	}

	unless(clang_getCursorKind(BASE) == CXCursor_DeclRefExpr)
		return 0;

	ae2f_ifezerr(REG = aclexpr_curreg_find(clang_getCursorReferenced(BASE), h_ctx))
		return 0;

	ae2f_ifezerr(p_reg = aclcur_get_node(REG, h_ctx))
		return 0;

	ae2f_ifezerr(p_ty = aclty_find(p_reg->m_ty, h_ctx))
		return 0;

	if(p_ty->m_class == ACLTY_PTR) {
		ae2f_ifezerr(*wr_id = aclexpr_ptrval(REG, h_ctx))
			return 0;

		*wr_ty = p_reg->m_ty;
		return 1;
	}

	/** a plain object value: the method receives its address. */
	ae2f_ifezerr(*wr_ty = aclty_mk_ptr(
				p_reg->m_ty, SpvStorageClassFunction, h_ctx))
		return 0;

	aclexpr_ptr_taken(REG, h_ctx);

	ae2f_ifezerr(*wr_id = aclcur_reg_addr(REG, h_ctx))
		return 0;

	return 1;
}

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_opn_call(
		const CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_extra,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_ascend,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		) {
	h_aclspv_ctx_t		ctx = rdwr_data->m_ctx;
	CXCursor		c_callee;
	aclsym_fn_t*		p_sym;
	aclspv_id_t		id_fn_ty;
	aclty_t* ae2f_restrict	p_fn_ty;
	aclspv_wrd_t		c_num;
	aclspv_wrd_t		c_total;
	aclspv_wrd_t		i;
	int			NARG;
	aclspv_bool_t		c_has_this;
	aclspv_bool_t		c_inline;
	aclspv_id_t		c_obj;
	aclty_id_t		c_obj_ty;

	if(c_cur.kind != CXCursor_CallExpr)
		return c_lbl_pass;

	c_callee = clang_getCursorReferenced(c_cur);

	if(clang_Cursor_isNull(c_callee)
			|| (c_callee.kind != CXCursor_FunctionDecl
				&& !aclutil_fn_is_method(c_callee)))
		return c_lbl_pass;

	if(aclutil_cursor_is_copy_move_ctor_call(c_cur)) {
		aclspv_id_t	id_val;
		aclty_id_t	ty_val;

		if(aclexpr_copy_ctor_value(c_cur, &id_val, &ty_val, ctx)) {
			rdwr_data->m_id_ret		= id_val;
			rdwr_data->m_tyid_current	= ty_val;
			rdwr_data->m_literal_flag	= 0;
			rdwr_extra->m_type_original	= ty_val;
			rdwr_extra->m_mask_literal	= 0;
			return c_lbl_ascend;
		}

		return c_lbl_pass;
	}

	p_sym = aclsym_fn_find(c_callee, ctx);

	/** not a cached symbol: a builtin or a header function. */
	unless(p_sym)
		return c_lbl_pass;

	/** only an emitted body can be called yet. */
	if(!(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED)))
		return c_lbl_fail;

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(
				c_callee
				, clang_getCursorType(c_callee)
				, ctx))
		return c_lbl_fail;

	ae2f_ifezerr(p_fn_ty = aclty_find(id_fn_ty, ctx))
		return c_lbl_fail;

	NARG = clang_Cursor_getNumArguments(c_cur);

	if(NARG < 0)
		return c_lbl_fail;

	c_num = (aclspv_wrd_t)NARG;
	c_has_this = aclutil_fn_has_this(c_callee);
	c_total = c_num + (c_has_this ? 1u : 0u);
	c_inline = (aclspv_bool_t)(
			(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED))
			&& s_should_inline(c_callee, ctx));

	if(!c_total || (c_has_this && !c_num)) {
		aclspv_id_t	id_call;
		aclspv_bool_t	is_pure = 0;
		aclspv_id_t	c_args[1];
		aclspv_wrd_t	c_nargs = 0;

		if(c_has_this) {
			ae2f_ifnezerr_strict(!s_method_object(
						c_cur, ctx, &c_obj, &c_obj_ty))
				return c_lbl_fail;

			c_args[0] = c_obj;
			c_nargs = 1;
		}

		if(c_inline) {
			if(!aclemit_inline_fn(
						c_callee
						, clang_getNullCursor()
						, c_args
						, c_nargs
						, &id_call
						, ctx))
				return c_lbl_fail;
		}
		else ae2f_ifnezerr(aclexpr_call_make(
					p_sym->m_id
					, p_fn_ty->m_info.m_fn.m_ret_id
					, c_args
					, c_nargs
					, &id_call
					, &is_pure
					, ctx))
			return c_lbl_fail;

		rdwr_data->m_id_ret		= id_call;
		rdwr_data->m_tyid_current	= p_fn_ty->m_info.m_fn.m_ret_id;
		rdwr_data->m_literal_flag	= 0;

		rdwr_extra->m_type_original	= p_fn_ty->m_info.m_fn.m_ret_id;
		rdwr_extra->m_mask_literal	= 0;

		return c_lbl_ascend;
	}

	{
		x_scale* ae2f_restrict		SCALE;
		aclspv_wrd_t* ae2f_restrict	BUF;

		SCALE = mk_scale_from_vec(
				&ctx->m_body.m_bootscale
				, (aclspv_wrd_t)(
					aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + c_total)
					+ (sizeof(aclexpr_opbincfg_extra)
						* (c_total + 1u))
					)
				);
		ae2f_ifezerr(SCALE) return c_lbl_fail;

		BUF = get_buf_from_scale2(aclspv_wrd_t, &ctx->m_body.m_bootscale, *SCALE);
		ae2f_ifezerr(BUF) return c_lbl_fail;

		BUF[ACLEXPR_OPBINCFG_NUMOPRND]		= c_total;
		BUF[ACLEXPR_OPBINCFG_SPECCONSTANTOP]	= p_sym->m_id;
		BUF[ACLEXPR_OPBINCFG_RESID]		= id_fn_ty;
		BUF[ACLEXPR_OPBINCFG_RESTYID]		= p_fn_ty->m_info.m_fn.m_ret_id;
		BUF[ACLEXPR_OPBINCFG_OPCODE]		= (aclspv_wrd_t)ACLEXPR_OPN_CALL;

		aclmemzero(
				&BUF[ACLEXPR_OPBINCFG_END + c_total]
				, (aclspv_wrd_t)(sizeof(aclexpr_opbincfg_extra)
					* (c_total + 1u))
				);

		if(c_inline) {
			aclexpr_opbincfg_extra* ae2f_restrict META
				= (aclexpr_opbincfg_extra* ae2f_restrict)(void*)
					(&BUF[ACLEXPR_OPBINCFG_END + c_total]);

			META[c_total].m_cursor = c_callee;
			META[c_total].m_state0
				= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_INLINE_CALL;
		}

		/** the scale is pushed by mk_scale_from_vec; the matching command
		 * counter is what OP_RECURSE would have pushed for an operator. */
		ae2f_ifnezerr_strict(s_aclmem_cgrow2(
					&ctx->m_body.m_cmdcountstck
					, rdwr_data->m_num_cmdcounter + 1
					, sizeof(aclexpr_cmdcounterstckelm)
					, ACLSPV_COMPILE_ALLOC_FAILED_EXPR))
			return c_lbl_fail;

		(void)++rdwr_data->m_num_cmdcounter;
		((aclexpr_cmdcounterstckelm* ae2f_restrict)
		 ctx->m_body.m_cmdcountstck.m_p)
			[rdwr_data->m_num_cmdcounter - 1].m_count_oprnd = 0;

		if(c_has_this) {
			ae2f_ifnezerr_strict(!s_method_object(
						c_cur, ctx, &c_obj, &c_obj_ty))
				return c_lbl_fail;

			BUF[ACLEXPR_OPBINCFG_END] = c_obj;
			((aclexpr_opbincfg_extra* ae2f_restrict)(void*)
			 (&BUF[ACLEXPR_OPBINCFG_END + c_total]))[0].m_type_original = c_obj_ty;

			((aclexpr_cmdcounterstckelm* ae2f_restrict)
			 ctx->m_body.m_cmdcountstck.m_p)
				[rdwr_data->m_num_cmdcounter - 1].m_count_oprnd = 1;
		}
	}

	for(i = 0; i < c_num; ++i) {
		const CXCursor ARG = clang_Cursor_getArgument(c_cur, (unsigned)i);

		ae2f_ifnezerr_strict(clang_Cursor_isNull(ARG))
			return c_lbl_fail;

		(void)aclutil_visitfromroot(ARG, aclexpr_visit, rdwr_data);

		aclctx_err_check(ctx)
			return c_lbl_fail;
	}

	return c_lbl_success;
}

#endif