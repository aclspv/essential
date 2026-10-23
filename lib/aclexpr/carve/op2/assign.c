#include "./assign.h"
#include <util/iddef.h>
#include <util/log.h>
#include <aclcur/common.h>
#include <aclexpr/curreg.h>
#include <aclcur/load.h>

aclspv_bool_t aclexpr_carve2_assign(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		) { 
	const aclspv_id_t REG = aclexpr_curreg_find(
			clang_getCursorReferenced(rdwr_lst_extra->m_cursor)
			, rdwr_data->m_ctx
			);

	h_aclcur_t p_reg = aclcur_get_node(REG, rdwr_data->m_ctx);

	ae2f_ifnezerr_strict(!REG) {
		err_puts("[carve2::assign] no register for the lvalue.\n");
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(p_reg) {
		err_puts("register not found as node\n");
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(aclcur_store_reg(
			REG
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_data->m_ctx
			)) {
		err_puts("[carve2::assign] assign cursor store has failed\n");
		return ACLEXPR_CARVE_NO;
	}

	/* implicit load: mirror memberwrite so the just-stored value is
	 * usable as an expression value -> chain assign works:
	 * a3 = (a4 = a3 << 4); */
	ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
			REG, rdwr_data->m_ctx, 0
			)) {
		err_puts("[carve2::assign] implicit load of the assigned value has failed\n");
		return ACLEXPR_CARVE_NO;
	}

	/* implicit load: yield the stored value so the assign result
	 * can serve as an expression value (chain assign:
	 * a3 = (a4 = a3 << 4)). mirrors memberwrite's implicit load. */
	ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
			REG, rdwr_data->m_ctx, 0
			)) {
	err_puts("[carve2::assign] implicit load of the assigned value has failed\n");
	return ACLEXPR_CARVE_NO;
	}

	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return ACLEXPR_CARVE_OK;
}
