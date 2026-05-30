/** 
 * @file repvec.c 
 * */

#include "../constant.h"
#include "../cxtp.h"
#include "../wrdemit.h"
#include "../literal.h"

ae2f_inline_f static struct aclutil_mk_constant_repvec_t ae2f_ccconst s_mkret(const aclspv_wrd_t c_id, const aclspv_wrd_t c_mask) {
	struct aclutil_mk_constant_repvec_t R;
	R.m_id = c_id;
	R.m_literal_mask = c_mask;
	return R;
}

struct aclutil_mk_constant_repvec_t	aclutil_mk_constant_repvec_id(
		const aclspv_id_t	c_id_ty,
		const aclspv_id_t c_val,
		const aclspv_wrd_t c_literal,
		h_aclspv_ctx_t h_ctx) {
	const u64_fast KEY = (u64_fast)(c_id_ty) | (((u64_fast)(c_val)) << (u64_fast)32);
	aclutil_constant* C = aclutil_mk_constant_node(KEY, h_ctx);

	aclspv_id_t	VEC_COUNT = aclutil_cxtp_get_veccfg(c_id_ty, h_ctx).m_num;

	const aclspv_wrd_t	OPCODE_PURE = ae2f_bll_sel_bool_naive1(
			aclspv_wrd_t
			, c_literal & (UTIL_LITERAL_CONSTANT | UTIL_LITERAL_LITERAL)
			, SpvOpSpecConstantComposite
			, SpvOpCompositeConstruct);

	const aclspv_wrd_t	OPCODE = opcode_to_wrd(OPCODE_PURE, VEC_COUNT + 2);


	x_aclspv_vec* ae2f_restrict const P_VEC =
			c_literal & (UTIL_LITERAL_CONSTANT | UTIL_LITERAL_LITERAL)
			? &h_ctx->m_section.m_types
			: &h_ctx->m_section.m_fnimpl;

	aclspv_wrd_t* ae2f_restrict const P_VWRD =
			c_literal & (UTIL_LITERAL_CONSTANT | UTIL_LITERAL_LITERAL)
			? &h_ctx->m_count.m_types
			: &h_ctx->m_count.m_fnimpl;

	const aclspv_wrd_t	ANCHOUR = *P_VWRD;

	dbg_puts("aclutil_mk_constant_repvec_id");

	ae2f_ifezerr(C) {
		h_ctx->m_id = ACLSPV_COMPILE_ALLOC_FAILED_VEC;
		return s_mkret(0, 0);
	}

	switch(VEC_COUNT) {
		case 0: return s_mkret(0, 0);
		case 1: return s_mkret(c_val, c_literal);
		default: break;
	}

	if(C->m_repvec)
		return s_mkret(C->m_repvec, C->m_repvec_literal_mask);

	ae2f_ifezerr(*P_VWRD = aclutil_emit_wrd(P_VEC, *P_VWRD + 2 + VEC_COUNT, 0)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_VEC;
		return s_mkret(0, 0);
	}

	ae2fpat_assertassume(get_wrd_of_vec(P_VEC));

	get_wrd_of_vec(P_VEC)[ANCHOUR + 0] = OPCODE;
	get_wrd_of_vec(P_VEC)[ANCHOUR + 1] = c_id_ty;
	get_wrd_of_vec(P_VEC)[ANCHOUR + 2] = C->m_repvec = h_ctx->m_id++;

	while(VEC_COUNT --> 0) {
		get_wrd_of_vec(P_VEC)[ANCHOUR + 3 + VEC_COUNT] = c_val;
	}

	return s_mkret(
			C->m_repvec
			, C->m_repvec_literal_mask = c_literal & ~(aclspv_id_t)(UTIL_LITERAL_LITERAL)
			);
}
