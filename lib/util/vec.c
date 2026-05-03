#include "./vec.h"
#include "./wrdemit.h"

#include <ae2f/bll.h>

aclspv_id_t aclutil_mkvec(
		aclspv_wrd_t				c_num_elm,
		aclspv_id_t				c_ty,
		const aclspv_id_t* ae2f_restrict	rd_elm,
		B_util_literal_t			c_literal,
		h_aclspv_ctx_t				h_ctx
		) {
	const aclspv_wrd_t	OPCODE = ae2f_bll_sel_bool_naive1(aclspv_wrd_t
			, c_literal & (UTIL_LITERAL_CONSTANT | UTIL_LITERAL_LITERAL)
			, SpvOpSpecConstantComposite
			, SpvOpConstantComposite);

	x_aclspv_vec* ae2f_restrict const P_VEC = ae2f_bll_sel_ptrbool_naive1(
			x_aclspv_vec*
			, c_literal & (UTIL_LITERAL_CONSTANT | UTIL_LITERAL_LITERAL)
			, &h_ctx->m_section.m_types
			, &h_ctx->m_section.m_fnimpl
			);

	aclspv_wrd_t* ae2f_restrict const P_VWRD = ae2f_bll_sel_ptrbool_naive1(
			aclspv_wrd_t*
			, c_literal & (UTIL_LITERAL_CONSTANT | UTIL_LITERAL_LITERAL)
			, &h_ctx->m_count.m_types
			, &h_ctx->m_count.m_fnimpl
			);


	ae2fpat_assertassume(rd_elm);
	ae2fpat_assertassume(h_ctx);

	ae2f_ifezerr(util_emit_wrd(P_VEC, *P_VWRD + 2 + c_num_elm, 0)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_VEC;
		return 0;
	}

	ae2fpat_assertassume(get_wrd_of_vec(P_VEC));

	get_wrd_of_vec(P_VEC)[0] = OPCODE;
	get_wrd_of_vec(P_VEC)[1] = c_ty;
	get_wrd_of_vec(P_VEC)[2] = h_ctx->m_id++;

	memcpy(&get_wrd_of_vec(P_VEC)[3], rd_elm, count_to_sz(c_num_elm));
	*P_VWRD += 3 + c_num_elm;

	return 0;
}
