/** boolean operator where two operands must be evaluated. */

#include "./bool0.h"
#include <aclconst/specop.h>

#include <util/log.h>
#include <ae2f/pat/dbg.h>

#include <aclcur/common.h>
#include <aclexpr/carve/lvalue.h>
#include <aclexpr/curreg.h>
#include <aclmem.h>
#include <aclval.h>
#include <aclty/simd.h>

ae2f_inline_f static ae2f_ccconst aclspv_wrd_t	s_opcvrt(const aclspv_wrd_t c_op) {
	switch(c_op) {
		case ACLEXPR_OP2_AND_A:
		case ACLEXPR_OP2_AND:
			return SpvOpBitwiseAnd;

		case ACLEXPR_OP2_AND_L:
			return SpvOpLogicalAnd;

		case ACLEXPR_OP2_OR_A:
		case ACLEXPR_OP2_OR:
			return SpvOpBitwiseOr;

		case ACLEXPR_OP2_OR_L:
			return SpvOpLogicalOr;

		case ACLEXPR_OP2_XOR_A:
		case ACLEXPR_OP2_XOR:
			return SpvOpBitwiseXor;

		default:
			return 0;
	}
}

ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_is_assign(const aclspv_wrd_t c) {
	switch(c) {
		case ACLEXPR_OP2_AND_A:
		case ACLEXPR_OP2_OR_A:
		case ACLEXPR_OP2_XOR_A:
			return 1;

		case ACLEXPR_OP2_AND_L:
		case ACLEXPR_OP2_OR_L:
		case ACLEXPR_OP2_AND:
		case ACLEXPR_OP2_OR:
		case ACLEXPR_OP2_XOR:
			return 0;

		default:
			ae2fpat_assertassume(0);
	}
}

ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_is_logical(const aclspv_wrd_t c) {
	switch(c) {
		case ACLEXPR_OP2_AND_L:
		case ACLEXPR_OP2_OR_L:
			return 1;

		case ACLEXPR_OP2_AND_A:
		case ACLEXPR_OP2_OR_A:
		case ACLEXPR_OP2_XOR_A:
		case ACLEXPR_OP2_AND:
		case ACLEXPR_OP2_OR:
		case ACLEXPR_OP2_XOR:
			return 0;

		default:
			ae2fpat_assertassume(0);
	}
}

static aclspv_id_t s_decide_cast(aclspv_id_t c_id0, aclspv_id_t c_id1, const h_aclspv_ctx_t h_ctx) {
	aclspv_id_t cls0, cls1;

	dbg_prefix(a);
	dbg_call(fprintf, (
				stderr, "s_decide_cast(%lu, %lu)\n"
				, (unsigned long)c_id0
				, (unsigned long)c_id1
				));

	if(c_id0 == c_id1)
		return (aclspv_id_t) c_id0;

	/** one is 0? */
	unless(c_id0 && c_id1)
		return c_id0 ? c_id0 : c_id1;

	if(c_id1 > c_id0) {
		c_id0 ^= c_id1;
		c_id1 ^= c_id0;
		c_id0 ^= c_id1;
	}

	cls0 = aclty_simd_class(c_id0, h_ctx);
	cls1 = aclty_simd_class(c_id1, h_ctx);

	ae2f_ifezerr(cls0 && cls1) {
		dbg_puts("s_decide_cast 0\n");
		return 0;
	}

	if(aclty_simd_chxg_cls(c_id0, cls1, h_ctx) != c_id1) {
		ae2f_ifneznrm(aclty_is_simd_class(c_id0))
			return c_id1;
		else	return 0;
	}

	ae2f_ifezerr(
			(cls0 == ACLSPV_ID_BOOL || aclid_is_int(cls0)) 
			&& (cls1 == ACLSPV_ID_BOOL || aclid_is_int(cls1))
			) {
		dbg_puts("s_decide_cast 0\n");
		return 0;
	}

	dbg_puts("s_decide_cast sint?");
	return aclid_is_sint(cls0) ? c_id0 : c_id1;
}

static aclexpr_carve_lbl_t s_iter(
		aclspv_wrd_t					c_idx_oprnd,
		aclspv_id_t					c_type_oprnd,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		)
{										
	if(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] != rdwr_lst_extra[c_idx_oprnd].m_type_original) {
		const aclutil_literal OLD_LITERAL = rdwr_lst_extra[c_idx_oprnd].m_literal;			
		aclspv_id_t OLD_ID 									
			= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + c_idx_oprnd];					
		rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + c_idx_oprnd] = (rdwr_data)->m_ctx->m_id++;		

		dbg_prefix(a);									
		dbg_call((void), fprintf(stderr, "ARITHMETIC CAST ON LOOP[%u]: %u %u\n"			
					, c_idx_oprnd
					, OLD_ID, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + c_idx_oprnd]	
					));

		switch(aclexpr_arithmetic_cast(
					OLD_ID
					, rdwr_lst_extra[c_idx_oprnd].m_type_original
					, rdwr_lst_extra[c_idx_oprnd].m_mask_literal
					, OLD_LITERAL
					, &rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + c_idx_oprnd]
					, c_type_oprnd
					, &rdwr_lst_extra[c_idx_oprnd].m_literal
					, (rdwr_data)->m_ctx
					))
		{
			default:
			case ACLEXPR_RET_FAILURE:
			case ACLEXPR_RET_NOT_THE_CASE:
				err_puts("[carve2::arithmetic_cast] OPITER"); return c_lbl_err;

			case ACLEXPR_RET_SUCCESS:
				rdwr_prv_extra->m_mask_literal &= 0;
				ae2f_fallthrough;

			case ACLEXPR_RET_SUCCESS_CONSTANT:
				rdwr_prv_extra->m_mask_literal &= 1;
				ae2f_fallthrough;

			case ACLEXPR_RET_SUCCESS_LITERAL:
				break;
		}
	}

	return c_lbl_success;
}

ae2f_inline_f static aclspv_id_t	s_ty_oprnd(
		aclspv_id_t	c_ty_origin,
		aclspv_bool_t	c_is_logical,
		h_aclspv_ctx_t	h_ctx
		) {
	return c_is_logical ? aclty_simd_chxg_cls(c_ty_origin, ACLSPV_ID_BOOL, h_ctx) : c_ty_origin;
}

ae2f_inline_f static aclspv_id_t	s_ty_out(
		aclspv_id_t	c_ty_origin,
		aclspv_bool_t	c_is_logical,
		h_aclspv_ctx_t	h_ctx
		) { 
	return s_ty_oprnd(c_ty_origin, c_is_logical, h_ctx);
}

ae2f_inline_f static ae2f_ccconst struct aclutil_ltrl_eval_t	s_eval2_ltrl(
		const aclutil_literal	c_oprndA,
		const aclutil_literal	c_oprndB,
		const enum SpvOp_	c_opcode,
		const enum ACLSPV_ID_	c_type
		)
{
	struct aclutil_ltrl_eval_t RET;

	switch((uintmax_t)c_type) {
#define	EXPR_TWO(L_oper, L_mem, c_case)					\
		case	c_case:						\
									\
		RET.m_ret.L_mem = c_oprndA.L_mem L_oper c_oprndB.L_mem;	\
		RET.m_found = 1					\

		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I32:
		case ACLSPV_ID_I64:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(&&, m_api_intmax, SpvOpLogicalAnd); return RET;
				EXPR_TWO(||, m_api_intmax, SpvOpLogicalOr); return RET;
				EXPR_TWO(&, m_api_intmax, SpvOpBitwiseAnd); return RET;
				EXPR_TWO(|, m_api_intmax, SpvOpBitwiseOr); return RET;
				EXPR_TWO(^, m_api_intmax, SpvOpBitwiseXor); return RET;
				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(&&, m_api_uintmax, SpvOpLogicalAnd); return RET;
				EXPR_TWO(||, m_api_uintmax, SpvOpLogicalOr); return RET;
				EXPR_TWO(&, m_api_uintmax, SpvOpBitwiseAnd); return RET;
				EXPR_TWO(|, m_api_uintmax, SpvOpBitwiseOr); return RET;
				EXPR_TWO(^, m_api_uintmax, SpvOpBitwiseXor); return RET;
				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		default:
			RET.m_ret.m_u64 = 0;
			RET.m_found = 0;
			return RET;
	}
	ae2f_unreachable();

#undef	EXPR_TWO
}

aclexpr_carve_lbl_t aclexpr_carve2_bool0(
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
		) { const aclspv_wrd_t OPNATIVE = s_opcvrt(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]); ae2fcc_unless(OPNATIVE) return c_lbl_continue; {

	const aclspv_bool_t ISASS = s_is_assign(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]);

	const aclspv_id_t TY_OPRND = s_ty_oprnd(s_decide_cast(
				rdwr_lst_extra[0].m_type_original
				, rdwr_lst_extra[1].m_type_original
				, (rdwr_data)->m_ctx
				), s_is_logical(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])
			, rdwr_data->m_ctx);

	if(s_is_logical(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]) && TY_OPRND == ACLSPV_ID_BOOL)
		return c_lbl_continue;

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID] = *rdwr_prv_idval = (rdwr_data)->m_ctx->m_id++;

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
		= s_ty_out(s_decide_cast(
					rdwr_lst_extra[0].m_type_original
					, rdwr_lst_extra[1].m_type_original
					, (rdwr_data)->m_ctx
					), s_is_logical(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])
				, rdwr_data->m_ctx);

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "restyid: %lu ; %lu\n"
				, (unsigned long)rdwr_prv_extra->m_type_original
				, (unsigned long)s_decide_cast(
					rdwr_lst_extra[0].m_type_original
					, rdwr_lst_extra[1].m_type_original
					, (rdwr_data)->m_ctx
					)
			  ));

	rdwr_prv_extra->m_mask_literal = rdwr_lst_extra[0].m_mask_literal & rdwr_lst_extra[1].m_mask_literal;

	ae2fpat_assertassume_strict(rdwr_lst_scale_buf);

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]) {
		err_puts("[carve2::bool] no result type");
		err_prefix(0);
		err_call((void), fprintf(stderr, "[carve2::bool] opcode: %u %u\n"
					, (unsigned)rdwr_lst_extra[0].m_type_original
					, (unsigned)rdwr_lst_extra[1].m_type_original));
		return c_lbl_err;
	}

	ae2f_ifnezerr_strict(s_iter(
				0, TY_OPRND
				, rdwr_data, rdwr_prv_extra
				, h_lst_scale, rdwr_lst_scale_buf
				, rdwr_lst_extra
				, c_lbl_success, c_lbl_err) == c_lbl_err)
		return c_lbl_err;

	ae2f_ifnezerr_strict(s_iter(
				1, TY_OPRND
				, rdwr_data, rdwr_prv_extra
				, h_lst_scale, rdwr_lst_scale_buf
				, rdwr_lst_extra
				, c_lbl_success, c_lbl_err) == c_lbl_err)
		return c_lbl_err;

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END] >= rdwr_data->m_ctx->m_id)
		return c_lbl_err;

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1] >= rdwr_data->m_ctx->m_id)
		return c_lbl_err;

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] = OPNATIVE;

	switch(rdwr_prv_extra->m_mask_literal) {
		case ACLUTIL_LITERAL_MASK_LITERAL: 
		case ACLUTIL_LITERAL_LITERAL:
			{
				struct aclutil_ltrl_eval_t EVAL_LIT 
					= s_eval2_ltrl(
							rdwr_lst_extra[0].m_literal
							, rdwr_lst_extra[1].m_literal
							, (enum SpvOp_)OPNATIVE
							,(enum ACLSPV_ID_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
						      );

				ae2f_ifnezerr_strict(EVAL_LIT.m_found) {
					dbg_puts(("The result is literal."));
					*r_ltrl = rdwr_prv_extra->m_literal = EVAL_LIT.m_ret;
					rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];


					ae2f_ifezerr(*rdwr_prv_idval = aclval_cfg_by_enum(EVAL_LIT.m_ret.m_api_uintmax, rdwr_data->m_ctx
								,(enum ACLSPV_ID_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID])) {
						err_puts("[carve2::bool] aclutil_mk_constant_val_by_enum");
						return c_lbl_err;
					}


					dbg_prefix();
					dbg_call((void), fprintf(stderr, "literal id: %u\n", *rdwr_prv_idval));
				} else {
					err_puts("[carve2::bool] EVAL_LIT");
					return c_lbl_err;
				}

				break;
			}

		case ACLUTIL_LITERAL_MASK_RT:
			/** runtime: the operation is a graph node, flushed by aclcur_emit(). */
			ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_op2(
						rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
						, (enum SpvOp_)OPNATIVE
						, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
						, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
						, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
						, rdwr_data->m_ctx))
				return c_lbl_err;
			break;

		case ACLUTIL_LITERAL_MASK_CONSTANT:
			ae2f_ifezerr(*rdwr_prv_idval = aclconst_specop(
						rdwr_lst_scale_buf
						, 6
						, rdwr_data->m_ctx)) return c_lbl_err;
			break;

		default:
			err_puts(("unknown literal"));
			ae2f_unreachable();
	}

	if(ISASS) {
		/** view-rooted (`a[i] &= x`), vector element (`v[i] &= x`) or plain
		 * variable: one shared store-back, keyed on what the l-value's own
		 * read recorded on extra[0]. */
		ae2f_ifezerr(aclexpr_lvalue_storeback(
					rdwr_lst_extra
					, *rdwr_prv_idval
					, rdwr_data->m_ctx
					, "[carve2::bool0] the l-value store back failed")) {
			return c_lbl_err;
		}
	}

	return c_lbl_success;
}}
