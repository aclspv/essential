#ifndef	aclexpr_visit_opn_call_h
#define	aclexpr_visit_opn_call_h

#include <util/scale.h>
#include <aclctx/err.h>

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/carve/opn/call.h>

#include <aclsym/fn.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <util/visitfromroot.h>

#include <aclmem.h>

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
ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_opn_call(
		const CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
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
	aclspv_wrd_t		i;
	int			NARG;

	if(c_cur.kind != CXCursor_CallExpr)
		return c_lbl_pass;

	c_callee = clang_getCursorReferenced(c_cur);

	if(clang_Cursor_isNull(c_callee) || c_callee.kind != CXCursor_FunctionDecl)
		return c_lbl_pass;

	p_sym = aclsym_fn_find(c_callee, ctx);

	/** not a cached symbol: a builtin or a header function. */
	unless(p_sym)
		return c_lbl_pass;

	/** only an emitted body can be called yet. */
	if(!(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED)))
		return c_lbl_fail;

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1(
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

	if(!c_num) {
		aclspv_id_t	id_call;
		aclspv_bool_t	is_pure = 0;

		ae2f_ifnezerr(aclexpr_call_make(
					p_sym->m_id
					, p_fn_ty->m_info.m_fn.m_ret_id
					, 0
					, 0
					, &id_call
					, &is_pure
					, ctx))
			return c_lbl_fail;

		rdwr_data->m_id_ret		= id_call;
		rdwr_data->m_tyid_current	= p_fn_ty->m_info.m_fn.m_ret_id;
		rdwr_data->m_literal_flag	= 0;

		return c_lbl_ascend;
	}

	{
		x_scale* ae2f_restrict		SCALE;
		aclspv_wrd_t* ae2f_restrict	BUF;

		SCALE = mk_scale_from_vec(
				&ctx->m_body.m_bootscale
				, (aclspv_wrd_t)(
					aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + c_num)
					+ (sizeof(aclexpr_opbincfg_extra) * c_num)
					)
				);
		ae2f_ifezerr(SCALE) return c_lbl_fail;

		BUF = get_buf_from_scale2(aclspv_wrd_t, &ctx->m_body.m_bootscale, *SCALE);
		ae2f_ifezerr(BUF) return c_lbl_fail;

		BUF[ACLEXPR_OPBINCFG_NUMOPRND]		= c_num;
		BUF[ACLEXPR_OPBINCFG_SPECCONSTANTOP]	= p_sym->m_id;
		BUF[ACLEXPR_OPBINCFG_RESID]		= id_fn_ty;
		BUF[ACLEXPR_OPBINCFG_RESTYID]		= p_fn_ty->m_info.m_fn.m_ret_id;
		BUF[ACLEXPR_OPBINCFG_OPCODE]		= (aclspv_wrd_t)ACLEXPR_OPN_CALL;

		aclmemzero(
				&BUF[ACLEXPR_OPBINCFG_END + c_num]
				, (aclspv_wrd_t)(sizeof(aclexpr_opbincfg_extra) * c_num)
				);

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
