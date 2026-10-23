#include "./incr.h"

#include <aclty.h>
#include <aclval.h>
#include <aclcur/common.h>
#include <aclexpr/carve/ptr.h>
#include <aclexpr/curreg.h>
#include <aclconst/specop.h>

#include <util/emitx.h>

#if	1

	aclspv_bool_t aclexpr_carve1_incr(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			)  {
		const aclspv_id_t	REG = aclexpr_curreg_find(
				clang_getCanonicalCursor(clang_getCursorDefinition(rdwr_lst_extra->m_cursor))
				, rdwr_data->m_ctx
				);

		aclutil_literal LTRL = {0, };
		aclspv_id_t	V;

		aclspv_wrd_t	O = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE];

		ae2f_ifnezerr_strict(!REG) {
			err_puts("[carve1::incr] no register for the lvalue.");
			return ACLEXPR_CARVE_NO;
		}

		/** `++p` / `--p` on a pointer is pointer arithmetic: a fatal error in
		 * the logical addressing model. */
		if(aclexpr_ptr_arith_fatal(rdwr_lst_extra->m_type_original, rdwr_data->m_ctx))
			return ACLEXPR_CARVE_NO;

		ae2f_ifezerr(aclid_is_int(aclty_get_veccfg(rdwr_lst_extra->m_type_original, rdwr_data->m_ctx).m_ty_elm)) {
			err_puts("[carve1::incr] no integer detected.");
			return ACLEXPR_CARVE_NO;
		}

		*rdwr_prv_extra = *rdwr_lst_extra;

		ae2f_ifezerr(V = aclcur_load_reg(REG, rdwr_data->m_ctx, &LTRL)) {
			err_puts("[carve1::incr] reading the lvalue has been failed.");
			return ACLEXPR_CARVE_NO;
		}

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

			ae2f_ifezerr(*rdwr_prv_idval = aclval_cfg_by_enum(
						LTRL.m_api_uintmax, rdwr_data->m_ctx
						, (enum ACLSPV_ID_)rdwr_lst_extra->m_type_original
						)) { 
				err_puts("[carve1::incr] getting value from cursor has been failed.");
				return ACLEXPR_CARVE_NO;
			}

			ae2f_ifezerr(aclcur_store_reg(
						REG, *rdwr_prv_idval, rdwr_data->m_ctx)) {
				err_puts("[carve1::incr] storing the incremented value has been failed.");
				return ACLEXPR_CARVE_NO;
			}

			return ACLEXPR_CARVE_OK;
		} else {
			const aclspv_wrd_t	LM = rdwr_lst_extra->m_mask_literal;
			aclspv_wrd_t		RO;

			const struct aclutil_cxtp_veccfg_t VCFG = aclty_get_veccfg(
					rdwr_lst_extra->m_type_original
					, rdwr_data->m_ctx);

			const aclspv_id_t	ONE = aclval_cfg_by_enum(
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
					return ACLEXPR_CARVE_NO;
			}

			switch(aclexpr_arithmetic_cast_non_literal(
						ONE, VCFG.m_ty_elm, LM
						, &ONE_CASTED, rdwr_lst_extra->m_type_original
						, rdwr_data->m_ctx)) {
				default:
				case ACLEXPR_RET_NOT_THE_CASE:
				case ACLEXPR_RET_FAILURE:
				case ACLEXPR_RET_SUCCESS_LITERAL:
					return ACLEXPR_CARVE_NO;

				case ACLEXPR_RET_SUCCESS:
					rdwr_prv_extra->m_mask_literal &= ACLUTIL_LITERAL_RT;
					ae2f_ifezerr(VP = aclcur_cfg_op2(
								VP, (enum SpvOp_)RO
								, rdwr_lst_extra->m_type_original
								, V, ONE_CASTED
								, rdwr_data->m_ctx))
						return ACLEXPR_CARVE_NO;
					break;

				case ACLEXPR_RET_SUCCESS_CONSTANT:
					{
						aclspv_wrd_t B[6]; 

						B[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
						B[1] = rdwr_lst_extra->m_type_original;
						B[2] = VP;
						B[3] = RO;
						B[4] = V;
						B[5] = ONE_CASTED;

						rdwr_prv_extra->m_mask_literal &= ACLUTIL_LITERAL_CONSTANT;

						ae2f_ifezerr(aclconst_specop(
									B
									, sizeof(B) / sizeof(B[0])
									, rdwr_data->m_ctx))
							return ACLEXPR_CARVE_NO;
					}
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
					return ACLEXPR_CARVE_NO;
			}

			ae2f_ifezerr(aclcur_store_reg(
						REG, VP, rdwr_data->m_ctx))
				return ACLEXPR_CARVE_NO;

			return ACLEXPR_CARVE_OK;
		}
	}

#endif
