#include "./assign.h"
#include <util/iddef.h>
#include <util/log.h>
#include <aclcur/common.h>
#include <aclexpr/curreg.h>
#include <aclcur/load.h>

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
	const aclspv_id_t REG = aclexpr_curreg_find(
			clang_getCursorReferenced(rdwr_lst_extra->m_cursor)
			, rdwr_data->m_ctx
			);

	h_aclcur_t p_reg = aclcur_get_node(REG, rdwr_data->m_ctx);

	ae2f_ifnezerr_strict(!REG) {
		err_puts("[carve2::assign] no register for the lvalue.\n");
		return c_lbl_err;
	}

	ae2f_ifezerr(p_reg) {
		err_puts("register not found as node\n");
		return c_lbl_err;
	}

	ae2f_ifezerr(aclcur_store_reg(
			REG
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_data->m_ctx
			)) {
		err_puts("[carve2::assign] assign cursor store has failed\n");
		return c_lbl_err;
	}

	/* implicit load: mirror memberwrite so the just-stored value is
	 * usable as an expression value -> chain assign works:
	 * a3 = (a4 = a3 << 4); */
	ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
			REG, rdwr_data->m_ctx, 0
			)) {
		err_puts("[carve2::assign] implicit load of the assigned value has failed\n");
		return c_lbl_err;
	}

	/* implicit load: yield the stored value so the assign result
	 * can serve as an expression value (chain assign:
	 * a3 = (a4 = a3 << 4)). mirrors memberwrite's implicit load. */
	ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
			REG, rdwr_data->m_ctx, 0
			)) {
	err_puts("[carve2::assign] implicit load of the assigned value has failed\n");
	return c_lbl_err;
	}

	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return c_lbl_success;
}}
