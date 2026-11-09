#include "./sin.h"

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <aclty.h>
#include <aclmem.h>

#include <util/scale.h>
#include <util/visitfromroot.h>

#include <string.h>

/** `__builtin_sinf(x)` (f32).
 *
 * a CallExpr has the callee as its first child and the argument as its
 * second. the callee must not enter the expression operand stream, so this
 * handler pushes the Sin scale and visits only the argument subtree; the
 * argument's OP0_ then carves this scale normally.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op1_sin(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	CXString		SPL;
	x_scale* ae2f_restrict	NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict VEC;
	aclspv_id_t		TY;
	CXCursor		ARG;
	enum CXChildVisitResult	RES;
	aclspv_bool_t		IS_SIN;

	if(c_cur.kind != CXCursor_CallExpr)
		return c_lbl_pass;

	SPL = clang_getCursorSpelling(c_cur);
	IS_SIN = !strcmp(SPL.data, "__builtin_sinf")
		|| !strcmp(SPL.data, "__builtin_elementwise_sin");
	clang_disposeString(SPL);

	unless(IS_SIN)
		return c_lbl_pass;

	ae2f_ifnezerr_strict(clang_Cursor_getNumArguments(c_cur) != 1)
		return c_lbl_fail;

	TY = aclutil_mk_cxtp_by_type2(
			clang_getCanonicalType(clang_getCursorType(c_cur))
			, rdwr_data->m_ctx);
	ae2f_ifezerr(TY) return c_lbl_fail;

	NEW_SCALE = mk_scale_from_vec(
			&rdwr_data->m_ctx->m_tmp.m_v1
			, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ sizeof(aclexpr_opbincfg_extra)
			);
	ae2f_ifezerr(NEW_SCALE) return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, &rdwr_data->m_ctx->m_tmp.m_v1, *NEW_SCALE);
	ae2f_ifezerr(VEC) return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP1_SIN;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= TY;
	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 1], sizeof(aclexpr_opbincfg_extra));
	/** the scale is pushed by mk_scale_from_vec; the matching command
	 * counter is what OP_RECURSE would have pushed for a normal operator.
	 * without it the operand count belongs to the outer scale. */
	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&rdwr_data->m_ctx->m_tmp.m_v2
				, rdwr_data->m_num_cmdcounter + 1
				, sizeof(aclexpr_cmdcounterstckelm)
				, ACLSPV_COMPILE_ALLOC_FAILED_EXPR))
		return c_lbl_fail;

	(void)++rdwr_data->m_num_cmdcounter;
	((aclexpr_cmdcounterstckelm* ae2f_restrict)
	 rdwr_data->m_ctx->m_tmp.m_v2.m_p)
		[rdwr_data->m_num_cmdcounter - 1].m_count_oprnd = 0;

	ARG = clang_Cursor_getArgument(c_cur, 0);
	ae2f_ifnezerr_strict(clang_Cursor_isNull(ARG))
		return c_lbl_fail;

	RES = aclutil_visitfromroot(ARG, aclexpr_visit, rdwr_data);
	(void)RES;

	/** the argument traversal can report Break as a normal stop after the
	 * operand was carved; only a context error is fatal here. */
	if(rdwr_data->m_ctx->m_err)
		return c_lbl_fail;

	return c_lbl_success;
}
