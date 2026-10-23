#include "./bool1.h"
#include <util/emitx.h>
#include <aclcur/common.h>
#include <.aclspv/util/mkname.h>

#include <aclty.h>

aclspv_bool_t aclexpr_carve2_bool1(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2)
		) { 
	/** op0 has been evaluated as boolean-like. */

	switch(aclexpr_arithmetic_cast(
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_lst_extra[1].m_type_original
			, rdwr_lst_extra[1].m_mask_literal
			, rdwr_lst_extra[1].m_literal
			, &rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, aclty_simd_chxg_cls( rdwr_lst_extra[1].m_type_original, ACLSPV_ID_BOOL, rdwr_data->m_ctx)
			, &rdwr_lst_extra[1].m_literal
			, rdwr_data->m_ctx)) {
		default:
			ae2f_unreachable();
		case ACLEXPR_RET_NOT_THE_CASE:
		case ACLEXPR_RET_FAILURE:
			err_puts("cast to boolean has been failed.");
			return ACLEXPR_CARVE_NO;

		case ACLEXPR_RET_SUCCESS:
		case ACLEXPR_RET_SUCCESS_CONSTANT:
		case ACLEXPR_RET_SUCCESS_LITERAL:
			fixme_puts("optimise it later.");
			break;
	}

	/** the branch-CUR block's tail is a branch: the auto-scan skips it, add it by hand.
	 * op1's value is defined in the branch-CUR block, so the store must land there
	 * BEFORE the jump; storing at the merge instead would reference a value that is
	 * undefined on the fallthrough edge (dominance violation on the merge's load). */
	ae2f_ifezerr(aclcur_store_reg(
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_data->m_ctx
			)) return ACLEXPR_CARVE_NO;

	/** now we can branch shit */
	ae2f_ifezerr(aclcur_cfg_jmp(rdwr_data->m_fnctx->m_branch, rdwr_data->m_ctx))
		return ACLEXPR_CARVE_NO;

	/** merge block. */
	ae2f_ifezerr(aclcur_cfg_lbl(rdwr_data->m_fnctx->m_branch, rdwr_data->m_ctx))
		return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
			, rdwr_data->m_ctx
			, 0
			)) return ACLEXPR_CARVE_NO;

	ae2f_ifnezerr_strict(aclcur_cfg_load_phi(
			*rdwr_prv_idval
			, rdwr_lst_extra[0].m_extra.m_bool1.m_parent
			, rdwr_data->m_ctx
			)) return ACLEXPR_CARVE_NO;

	rdwr_prv_extra->m_mask_literal	= 0;
	rdwr_prv_extra->m_type_original	= ACLSPV_ID_BOOL;

	ae2fpat_assertassume_strict(*rdwr_prv_idval);
	ae2fpat_assertassume_strict(rdwr_lst_extra[0].m_extra.m_bool1.m_parent);
	ae2fpat_assertassume_strict(rdwr_lst_extra[1].m_extra.m_bool1.m_parent);
	ae2fpat_assertassume_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]);

	_aclutil_mkname_dbg(
			L, *rdwr_prv_idval
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == ACLEXPR_OP2_AND_L ? "and" : "or"
			, *rdwr_data->m_ctx
			, rdwr_data->m_ctx->m_tmp.m_v0);

	return ACLEXPR_CARVE_OK;
}
