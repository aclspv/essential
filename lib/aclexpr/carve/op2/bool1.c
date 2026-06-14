#include "./bool1.h"
#include <util/emitx.h>
#include <aclcursor/vardecl.h>
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

	ae2f_ifnezerr(rdwr_data->m_ctx->m_err = aclcursor_vardecl_evuclbl(
			rdwr_data->m_ctx->m_num_cursor
			, rdwr_data->m_ctx->m_cursors.m_p
			, rdwr_data->m_ctx
			, rdwr_data->m_fnctx->m_var_stage + 1))
		return c_lbl_err;

	/** now we can branch shit */
	ae2f_ifezerr(rdwr_data->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
			&rdwr_data->m_ctx->m_section.m_fnimpl
			, rdwr_data->m_ctx->m_count.m_fnimpl
			, SpvOpBranch
			, rdwr_data->m_fnctx->m_branch)) {
		err_puts("branch fucked");
		return c_lbl_err;
	}

	/** now we can label this shit */
	ae2f_ifezerr(rdwr_data->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
			&rdwr_data->m_ctx->m_section.m_fnimpl
			, rdwr_data->m_ctx->m_count.m_fnimpl
			, SpvOpLabel
			, rdwr_data->m_fnctx->m_branch)) {
		err_puts("label fucked");
		return c_lbl_err;
	}

	ae2f_ifezerr(rdwr_data->m_ctx->m_count.m_fnimpl = aclutil_emitx_7(
			&rdwr_data->m_ctx->m_section.m_fnimpl
			, rdwr_data->m_ctx->m_count.m_fnimpl
			, SpvOpPhi
			, ACLSPV_ID_BOOL
			, *rdwr_prv_idval = rdwr_data->m_ctx->m_id++
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
			, rdwr_lst_extra[0].m_extra.m_bool1.m_parent
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
			, rdwr_lst_extra[1].m_extra.m_bool1.m_parent
			)) {
		err_puts("phi fucked.");
		return c_lbl_err;
	}

	rdwr_prv_extra->m_mask_literal	= 0;
	rdwr_prv_extra->m_type_original	= ACLSPV_ID_BOOL;

	ae2fpat_assertassume_strict(*rdwr_prv_idval);
	ae2fpat_assertassume_strict(rdwr_lst_extra[0].m_extra.m_bool1.m_parent);
	ae2fpat_assertassume_strict(rdwr_lst_extra[1].m_extra.m_bool1.m_parent);
	ae2fpat_assertassume_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]);
	ae2fpat_assertassume_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]);

	_aclutil_mkname_dbg(
			L, *rdwr_prv_idval
			, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == ACLEXPR_OP2_AND_L ? "and" : "or"
			, *rdwr_data->m_ctx
			, rdwr_data->m_ctx->m_tmp.m_v0);

	return c_lbl_success;
} return c_lbl_continue; }
