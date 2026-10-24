#if	__ACLSPV_MATH_NR

#include <aclty.h>
#include "./rsqrtf_nr.h"

aclexpr_carve_lbl_t aclexpr_carve_op1_rsqrtf_nr(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		h_scale_t					h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		aclutil_literal* ae2f_restrict			r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		) {
	aclspv_id_t f32_like, u32_like;

	rdwr_prv_extra->m_mask_literal = (aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL;

	ae2f_ifezerr(aclty_is_simd(rdwr_lst_extra[0].m_type_original, rdwr_data->m_ctx)) {
		return c_lbl_err;
	}

	ae2f_ifezerr(f32_like = aclty_simd_chxg_cls(
			rdwr_lst_extra[0].m_type_original
			, aclid_mk_default_id(ACLSPV_ID_F32, rdwr_data->m_ctx)
			, rdwr_data->m_ctx
			)) return c_lbl_err;

	ae2f_ifezerr(u32_like = aclty_simd_chxg_cls(
			rdwr_lst_extra[0].m_type_original
			, aclid_mk_default_id(ACLSPV_ID_U32, rdwr_data->m_ctx)
			, rdwr_data->m_ctx
			)) return c_lbl_err;


}
#else
typedef int dummy_t;
#endif
