#include "./shuffle.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/op.h>
#include <aclexpr/util/unwrap.h>

#include <aclcur/opX.h>
#include <aclattr/getkinder.h>

#include <util/scale.h>

/** descend to the integer literal backing a shuffle component. */
static enum CXChildVisitResult s_shuffle_findint(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_out
		) {
	CXCursor* ae2f_restrict p = rdwr_out;

	if(c_cur.kind == CXCursor_IntegerLiteral) {
		*p = c_cur;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Recurse;
}

/** one mask component: a constant integer, recovered off the cursor. */
static aclspv_bool_t s_shuffle_component(
		CXCursor		c_cur,
		aclspv_wrd_t* ae2f_restrict r_val
		) {
	CXCursor	INT = aclexpr_expr_unwrap(c_cur);
	CXEvalResult	EVAL;
	int		VAL;

	if(INT.kind != CXCursor_IntegerLiteral)
		(void)clang_visitChildren(INT, s_shuffle_findint, &INT);

	if(INT.kind != CXCursor_IntegerLiteral)
		return 1;

	EVAL = clang_Cursor_Evaluate(INT);
	ae2f_ifezerr(EVAL) return 1;

	if(clang_EvalResult_getKind(EVAL) != CXEval_Int) {
		clang_EvalResult_dispose(EVAL);
		return 1;
	}

	VAL = clang_EvalResult_getAsInt(EVAL);
	clang_EvalResult_dispose(EVAL);

	if(VAL < 0)
		return 1;

	*r_val = (aclspv_wrd_t)VAL;
	return 0;
}

/** `__builtin_shufflevector(v1, v2, ...)` -> OpVectorShuffle.
 *
 * the two vectors are the operands the cascade produced; the indices are
 * literal components recovered from the operator cursor (stashed on
 * extra[2] by the visit).
 * */
aclspv_bool_t aclexpr_carve_opn_shuffle(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		aclutil_literal* ae2f_restrict			r_ltrl
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	const aclspv_id_t	V1 = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
	const aclspv_id_t	V2 = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];
	const CXCursor		CUR = rdwr_lst_extra[2].m_cursor;
	CXCursor		CH[__ACLSPV_MAX_OPRND];
	aclspv_wrd_t		COMPS[__ACLSPV_MAX_OPRND];
	struct aclattr_getkinder	K;
	struct aclutil_cxtp_veccfg_t	V;
	aclspv_wrd_t		NUM_COMP;
	aclspv_wrd_t		i;
	aclspv_id_t		ID;

	ae2f_ifezerr(TY)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(V1)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(V2)	return ACLEXPR_CARVE_NO;

	ae2f_ifnezerr_strict(clang_Cursor_isNull(CUR))
		return ACLEXPR_CARVE_NO;

	/** children: v1, v2, then one constant per result component. */
	K.m_cursor	= CH;
	K.m_res		= CXChildVisit_Continue;
	K.m_num_wanted	= __ACLSPV_MAX_OPRND;
	K.m_num_grepped	= 0;

	(void)clang_visitChildren(CUR, s_aclattr_getkinder_visit, &K);

	ae2f_ifnezerr_strict(K.m_num_grepped < 3)
		return ACLEXPR_CARVE_NO;

	NUM_COMP = K.m_num_grepped - 2;

	/** the result vector length is the mask length. */
	V = aclty_get_veccfg(TY, CTX);
	ae2f_ifnezerr_strict(V.m_num != NUM_COMP)
		return ACLEXPR_CARVE_NO;

	for(i = 0; i < NUM_COMP; ++i) {
		if(s_shuffle_component(CH[2 + i], &COMPS[i]))
			return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(ID = aclcur_cfg_shuffle(
				0
				, TY
				, V1
				, V2
				, COMPS
				, NUM_COMP
				, CTX))
		return ACLEXPR_CARVE_NO;

	*rdwr_prv_idval = ID;
	rdwr_prv_extra->m_type_original = TY;
	rdwr_prv_extra->m_mask_literal = 0;
	rdwr_prv_extra->m_literal.m_api_uintmax = 0;
	*r_ltrl = rdwr_prv_extra->m_literal;

	return ACLEXPR_CARVE_OK;
}
