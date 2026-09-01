#include "./incr.h"

#include <aclty.h>
#include <aclcursor.h>

#include <util/constant.h>
#include <util/emitx.h>

#if	1

ae2f_inline_f	ae2f_ccconst static aclspv_bool_t	s_check(aclspv_wrd_t	c_op) {
	switch(c_op) {
		case ACLEXPR_OP1_POSTDEC:
		case ACLEXPR_OP1_POSTINC:
		case ACLEXPR_OP1_PREDEC:
		case ACLEXPR_OP1_PREINC:
			return 1;

		default:
			return 0;
	}
}

aclexpr_carve_lbl_t aclexpr_carve1_incr(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		aclutil_literal* ae2f_restrict	r_ltrl,
		aclexpr_carve_lbl_t		c_lbl_continue,
		aclexpr_carve_lbl_t		c_lbl_success,
		aclexpr_carve_lbl_t		c_lbl_err
		) { unless(s_check(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])) return c_lbl_continue; {
	const aclspv_wrd_t	CURIDX = aclutil_find_cursor(
			rdwr_data->m_ctx->m_num_cursor
			, rdwr_data->m_ctx->m_cursors.m_p
			, clang_getCanonicalCursor(clang_getCursorDefinition(rdwr_lst_extra->m_cursor))
			);

	x_aclcursor	* ae2f_restrict C = ((x_aclcursor* ae2f_restrict)rdwr_data->m_ctx->m_cursors.m_p) + CURIDX;

	aclutil_literal LTRL;
	aclspv_id_t	V;

	aclspv_wrd_t	O = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE];

	ae2f_ifnezerr_strict(CURIDX >= rdwr_data->m_ctx->m_num_cursor) {
		err_puts("[carve1::incr] CURIDX is out of bound.");
		return c_lbl_err;
	}

	ae2f_ifezerr(aclid_is_int(aclty_get_veccfg(rdwr_lst_extra->m_type_original, rdwr_data->m_ctx).m_ty_elm)) {
		err_puts("[carve1::incr] no integer detected.");
		return c_lbl_err;
	}

	*rdwr_prv_extra = *rdwr_lst_extra;


	V = aclcursor_vardecl_assign(C, rdwr_data->m_fnctx, 0, 0, 0
			, 0, &LTRL);

	if(rdwr_lst_extra->m_mask_literal & (aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL) {
		switch(O) {
			case ACLEXPR_OP1_PREDEC:
				r_ltrl->m_api_uintmax = LTRL.m_api_uintmax - 1;
				break;
			case ACLEXPR_OP1_PREINC:
				r_ltrl->m_api_uintmax = LTRL.m_api_uintmax + 1;
				break;
			case ACLEXPR_OP1_POSTDEC:
				r_ltrl->m_api_uintmax = LTRL.m_api_uintmax--;
				break;
			case ACLEXPR_OP1_POSTINC:
				r_ltrl->m_api_uintmax = LTRL.m_api_uintmax++;
				break;
			default:
				ae2f_unreachable();
		}

		ae2f_ifezerr(*rdwr_prv_idval = aclcursor_vardecl_assign(
					C, rdwr_data->m_fnctx, 0
					, rdwr_lst_extra->m_type_original
					, ACLUTIL_LITERAL_LITERAL
					, &LTRL, 0
					)) { 
			err_puts("[carve1::incr] getting value from cursor has been failed.");
			return c_lbl_err;
		}

		return c_lbl_success;
	} else {
		const aclspv_wrd_t	LM = rdwr_lst_extra->m_mask_literal;
		aclspv_wrd_t		RO;

		const struct aclutil_cxtp_veccfg_t VCFG = aclty_get_veccfg(
				rdwr_lst_extra->m_type_original
				, rdwr_data->m_ctx);

		const aclspv_id_t	ONE = aclutil_mk_constant_val_by_enum(
				1u, rdwr_data->m_ctx
				, (enum ACLSPV_ID_)VCFG.m_ty_elm
				);
		aclspv_id_t	VP = rdwr_data->m_ctx->m_id++;
		aclspv_id_t	ONE_CASTED;

		switch(O) {
			case ACLEXPR_OP1_PREINC:
			case ACLEXPR_OP1_POSTINC:
				RO = SpvOpIAdd;
				break;

			case ACLEXPR_OP1_POSTDEC:
			case ACLEXPR_OP1_PREDEC:
				RO = SpvOpISub;
				break;

			default:
				return c_lbl_err;
		}

		switch(aclexpr_arithmetic_cast_non_literal(
					ONE, VCFG.m_ty_elm, LM
					, &ONE_CASTED, rdwr_lst_extra->m_type_original
					, rdwr_data->m_ctx)) {
			default:
			case ACLEXPR_RET_NOT_THE_CASE:
			case ACLEXPR_RET_FAILURE:
			case ACLEXPR_RET_SUCCESS_LITERAL:
				return c_lbl_err;

			case ACLEXPR_RET_SUCCESS:
				rdwr_prv_extra->m_mask_literal &= ACLUTIL_LITERAL_RT;
				ae2f_ifezerr(rdwr_data->m_ctx->m_count.m_fnimpl = aclutil_emitx_5(
							&rdwr_data->m_ctx->m_section.m_fnimpl
							, rdwr_data->m_ctx->m_count.m_fnimpl
							, RO, rdwr_lst_extra->m_type_original
							, VP
							, V, ONE_CASTED))
					return c_lbl_err;
				break;

			case ACLEXPR_RET_SUCCESS_CONSTANT:
				rdwr_prv_extra->m_mask_literal &= ACLUTIL_LITERAL_CONSTANT;
				ae2f_ifezerr(rdwr_data->m_ctx->m_count.m_types = aclutil_emitx_6(
							&rdwr_data->m_ctx->m_section.m_types
							, rdwr_data->m_ctx->m_count.m_types
							, SpvOpSpecConstantOp
							, rdwr_lst_extra->m_type_original
							, VP, RO
							, V, ONE_CASTED))
					return c_lbl_err;
				break;
		}

		switch(O) {
			case ACLEXPR_OP1_PREINC:
			case ACLEXPR_OP1_PREDEC:
				*rdwr_prv_idval = VP;
				break;

			case ACLEXPR_OP1_POSTDEC:
			case ACLEXPR_OP1_POSTINC:
				*rdwr_prv_idval = V;
				break;
			default:
				return c_lbl_err;
		}

		ae2f_ifnezerr_strict(VP != aclcursor_vardecl_assign(
					C, rdwr_data->m_fnctx
					, VP, rdwr_lst_extra->m_type_original
					, LM
					, 0, 0))
			return c_lbl_err;

		return c_lbl_success;
	}
} ae2f_unreachable(); }

#endif
