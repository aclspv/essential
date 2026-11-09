#include "./glslmath.h"

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <aclmem.h>
#include <aclutil/wrdemit.h>

#include <util/scale.h>
#include <util/visitfromroot.h>

#include <string.h>

/** spelling -> opcode and expected operand type.
 *
 * the expected type is a property of the builtin's signature, not of the
 * cursor: `*f` is f32, `*f16` is f16, and the generic elementwise spelling
 * leaves it 0 (the carve then expects a simd float). */
static const struct s_glslmath {
	const char*	m_name;
	aclspv_wrd_t	m_op;
	aclspv_id_t	m_expected;
} S_MATH[] = {
	{ "__builtin_sinf",		ACLEXPR_OP1_SIN, ACLSPV_ID_F32 },
	{ "__builtin_cosf",		ACLEXPR_OP1_COS, ACLSPV_ID_F32 },
	{ "__builtin_tanf",		ACLEXPR_OP1_TAN, ACLSPV_ID_F32 },

	{ "__builtin_sinf16",		ACLEXPR_OP1_SIN, ACLSPV_ID_F16 },
	{ "__builtin_cosf16",		ACLEXPR_OP1_COS, ACLSPV_ID_F16 },
	{ "__builtin_tanf16",		ACLEXPR_OP1_TAN, ACLSPV_ID_F16 },

	{ "__builtin_elementwise_sin",	ACLEXPR_OP1_SIN, 0 },
	{ "__builtin_elementwise_cos",	ACLEXPR_OP1_COS, 0 },
	{ "__builtin_elementwise_tan",	ACLEXPR_OP1_TAN, 0 },
};

/** a CallExpr has the callee as its first child and the argument as its
 * second. the callee must not enter the expression operand stream, so this
 * handler pushes the math scale and visits only the argument subtree; the
 * argument's OP0_ then carves this scale normally.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op1_glsl_math(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	CXString		SPL;
	x_scale* ae2f_restrict	NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict VEC;
	aclspv_wrd_t		OP = 0;
	aclspv_id_t		EXPECTED = 0;
	size_t			i;
	CXCursor		ARG;
	enum CXChildVisitResult	RES;

	if(c_cur.kind != CXCursor_CallExpr)
		return c_lbl_pass;

	SPL = clang_getCursorSpelling(c_cur);

	for(i = 0; i < sizeof(S_MATH) / sizeof(S_MATH[0]); ++i) {
		if(!strcmp(SPL.data, S_MATH[i].m_name)) {
			OP = S_MATH[i].m_op;
			EXPECTED = S_MATH[i].m_expected;
			break;
		}
	}

	clang_disposeString(SPL);

	unless(OP)
		return c_lbl_pass;

	ae2f_ifnezerr_strict(clang_Cursor_getNumArguments(c_cur) != 1)
		return c_lbl_fail;

	NEW_SCALE = mk_scale_from_vec(
			&rdwr_data->m_ctx->m_tmp.m_v1
			, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ sizeof(aclexpr_opbincfg_extra)
			);
	ae2f_ifezerr(NEW_SCALE) return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, &rdwr_data->m_ctx->m_tmp.m_v1, *NEW_SCALE);
	ae2f_ifezerr(VEC) return c_lbl_fail;

	/** the builtin's signature fixes the expected operand type; 0 means
	 * generic (a simd float is expected). the carve does the conversion. */
	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= OP;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= EXPECTED;
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
