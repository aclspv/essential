#include "./bool1.h"
#include <util/emitx.h>
#include <aclcur/common.h>
#include <.aclspv/util/mkname.h>

aclexpr_carve_lbl_t aclexpr_carve2_bool1(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		ae2f_unused aclutil_literal* ae2f_restrict	r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		) { const aclspv_wrd_t OPC = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]; if(OPC == ACLEXPR_OP2_AND_L || OPC == ACLEXPR_OP2_OR_L)  {
	/** op0 has been evaluated as boolean-like. */

	switch(aclexpr_arithmetic_cast(
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_lst_extra[1].m_type_original
			, rdwr_lst_extra[1].m_mask_literal
			, rdwr_lst_extra[1].m_literal
			, &rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, ACLSPV_ID_BOOL
			, &rdwr_lst_extra[1].m_literal
			, rdwr_data->m_ctx)) {
		default:
			ae2f_unreachable();
		case ACLEXPR_RET_NOT_THE_CASE:
		case ACLEXPR_RET_FAILURE:
			err_puts("cast to boolean has been failed.");
			return c_lbl_err;

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
			)) return c_lbl_err;

	/** now we can branch shit */
	ae2f_ifezerr(aclcur_cfg_jmp(rdwr_data->m_fnctx->m_branch, rdwr_data->m_ctx))
		return c_lbl_err;

	/** merge block. */
	ae2f_ifezerr(aclcur_cfg_lbl(rdwr_data->m_fnctx->m_branch, rdwr_data->m_ctx))
		return c_lbl_err;

	ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
			, rdwr_data->m_ctx
			, 0
			)) return c_lbl_err;

	ae2f_ifnezerr_strict(aclcur_cfg_load_phi(
			*rdwr_prv_idval
			, rdwr_lst_extra[0].m_extra.m_bool1.m_parent
			, rdwr_data->m_ctx
			)) return c_lbl_err;

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

	return c_lbl_success;
} return c_lbl_continue; }
