#include "./assign.h"
#include <util/iddef.h>
#include <aclcursor.h>

aclexpr_carve_lbl_t aclexpr_carve2_assign(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		aclutil_literal* ae2f_restrict			r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		) { ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] != ACLEXPR_OP2_ASSIGN) return c_lbl_continue; {
	const aclspv_wrdcount_t CIDX = aclutil_find_cursor(
			rdwr_data->m_ctx->m_num_cursor
			, rdwr_data->m_ctx->m_cursors.m_p
			, clang_getCursorReferenced(rdwr_lst_extra->m_cursor));


	x_aclcursor* const ae2f_restrict C = &((x_aclcursor* ae2f_restrict)rdwr_data->m_ctx->m_cursors.m_p)[CIDX];

	ae2f_ifnezerr_strict(CIDX >= rdwr_data->m_ctx->m_num_cursor) {
		err_puts("[carve2::assign] CIDX assert");
		return c_lbl_err;
	}

	ae2f_ifezerr(*rdwr_prv_idval = aclcursor_vardecl_assign(
			C, rdwr_data->m_ctx
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_lst_extra[1].m_type_original
			, rdwr_lst_extra[1].m_mask_literal
			, &rdwr_lst_extra[1].m_literal
			, r_ltrl
			)) {
		err_puts("[carve2::assign] assign cursor has failed");
		return c_lbl_err;
	}

	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return c_lbl_success;
}}
