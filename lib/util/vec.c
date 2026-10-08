#include "./vec.h"

#include <util/wrdemit.h>
#include <aclty.h>

#include <ae2f/bll.h>
#include <ae2f/pat/dbg.h>

aclspv_id_t aclutil_mkvec(
		aclspv_wrd_t				c_num_elm,
		aclspv_id_t				c_ty,
		const aclspv_id_t* ae2f_restrict	rd_elm,
		aclutil_ltrlmask_t			c_literal,
		h_aclspv_ctx_t				h_ctx
		) {
	const aclspv_wrd_t	OPCODE_PURE = ae2f_bll_sel_bool_naive1(
			aclspv_wrd_t
			, c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)
			, SpvOpSpecConstantComposite
			, SpvOpCompositeConstruct);

	const aclspv_wrd_t	OPCODE = aclspv_opcode_to_wrd(OPCODE_PURE, c_num_elm + 2);

	x_aclspv_vec* ae2f_restrict const P_VEC =
			c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)
			? &h_ctx->m_section.m_types
			: &h_ctx->m_section.m_fnimpl;

	aclspv_wrd_t* ae2f_restrict const P_VWRD =
			c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)
			? &h_ctx->m_count.m_types
			: &h_ctx->m_count.m_fnimpl;

	const aclspv_wrd_t	ANCHOUR = *P_VWRD;

	const aclspv_wrd_t	TY = aclty_mk_vec(c_ty, c_num_elm, h_ctx);

	ae2fpat_assertassume(rd_elm);
	ae2fpat_assertassume(h_ctx);

	ae2f_ifezerr(TY) {
		return 0;
	}

	ae2f_ifezerr(*P_VWRD = aclutil_emit_wrd(P_VEC, *P_VWRD + 2 + c_num_elm, 0)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_VEC;
		return 0;
	}

	ae2fpat_assertassume(aclutil_get_wrd_of_vec(P_VEC));

	aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 0] = OPCODE;
	aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 1] = TY;
	aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 2] = h_ctx->m_id++;

	{
		aclspv_wrd_t	C = c_num_elm;
		while(C --> 0) {
			aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 3 + C] = rd_elm[C];
		}
	}

	return aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 2];
}
