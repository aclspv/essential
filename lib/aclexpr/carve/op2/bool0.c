/** boolean operator where two operands must be evaluated. */

#include "./bool0.h"
#include <util/constant.h>
#include <util/wrdemit.h>
#include <ae2f/pat/dbg.h>

#include <aclcursor.h>
#include <aclmem.h>

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

static aclspv_wrd_t s_decide_cast(const enum ACLSPV_ID_ c_id0, const enum ACLSPV_ID_ c_id1, const h_aclspv_ctx_t h_ctx) {
	/** one is 0? */
	unless(c_id0 & c_id1)
		return ae2f_bll_sel_bool_naive1(B_id_default_t, c_id0, (B_id_default_t) c_id0, (B_id_default_t) c_id1);

	unless(c_id0 ^ c_id1)
		return (B_id_default_t) c_id0;

	unless(aclid_is_arithmetic((B_id_default_t) c_id0, h_ctx) & aclid_is_arithmetic((B_id_default_t) c_id1, h_ctx))
		return 0;

	if(aclty_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num ^ aclty_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num) {
		unless(aclty_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id1;
		} else unless(aclty_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id0;
		} return 0;
	}

	unless(aclid_is_vectorisable((B_id_default_t) c_id0)) {
		return ae2f_bll_sel_bool1(B_id_default_t, s_decide_cast(
					(enum ACLSPV_ID_) aclty_get_veccfg((B_id_default_t) c_id0, h_ctx).m_ty_elm
					, (enum ACLSPV_ID_) aclty_get_veccfg((B_id_default_t) c_id1, h_ctx).m_ty_elm
					, h_ctx
					), (B_id_default_t) c_id0, (B_id_default_t) c_id1);
	}

	unless(aclid_is_int((B_id_default_t) c_id0) & aclid_is_int((B_id_default_t) c_id1))
		return 0;

	return ae2f_bll_sel_bool1(aclspv_wrd_t, aclid_is_sint((aclspv_wrd_t)c_id0), (aclspv_wrd_t)c_id0, (aclspv_wrd_t)c_id1);
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

static aclspv_id_t	s_ty_oprnd(
		aclspv_id_t	c_ty_origin,
		aclspv_bool_t	c_is_logical,
		h_aclspv_ctx_t	h_ctx
		) {
	unless(c_is_logical)
		return c_ty_origin; 

	{
		struct aclutil_cxtp_veccfg_t V = aclty_get_veccfg(c_ty_origin, h_ctx);
		return aclutil_mk_cxtp_vec(ACLSPV_ID_BOOL, V.m_num, h_ctx);
	}
}

static aclspv_id_t	s_ty_out(
		aclspv_id_t	c_ty_origin,
		aclspv_bool_t	c_is_logical,
		h_aclspv_ctx_t	h_ctx
		) { unless(c_is_logical) return c_ty_origin; else {
	struct aclutil_cxtp_veccfg_t V = aclty_get_veccfg(c_ty_origin, h_ctx);
	return aclutil_mk_cxtp_vec(ACLSPV_ID_BOOL, V.m_num, h_ctx);
}}

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
				(enum ACLSPV_ID_) rdwr_lst_extra[0].m_type_original
				, (enum ACLSPV_ID_) rdwr_lst_extra[1].m_type_original
				, (rdwr_data)->m_ctx
				), s_is_logical(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])
			, rdwr_data->m_ctx);

	if(s_is_logical(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]) && TY_OPRND == ACLSPV_ID_BOOL)
		return c_lbl_continue;

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID] = *rdwr_prv_idval = (rdwr_data)->m_ctx->m_id++;

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
		= s_ty_out(s_decide_cast(
					(enum ACLSPV_ID_) rdwr_lst_extra[0].m_type_original
					, (enum ACLSPV_ID_) rdwr_lst_extra[1].m_type_original
					, (rdwr_data)->m_ctx
					), s_is_logical(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])
				, rdwr_data->m_ctx);

	rdwr_prv_extra->m_mask_literal = 3;

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


					ae2f_ifezerr(*rdwr_prv_idval = aclutil_mk_constant_val_by_enum(EVAL_LIT.m_ret.m_api_uintmax, rdwr_data->m_ctx
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
#define	XORSWAP(a, b)	{ (a) ^= (b); (b) ^= (a); (a) ^= (b); }
			/** 
			 * here, operation is not constant.
			 *
			 * current
			 * [1] OpSpecConstant opcode (ignorance required)
			 * [2] result type
			 * [3] result id
			 * [4] literal opcode (opspecconstant evaluation)
			 * ...
			 *
			 *
			 * we want
			 * [2] opcode with operand count
			 * [3] result type
			 * [4] result id
			 * */

			/** 
			 * swap [2] and [4]
			 *
			 * after this will
			 * [2] literal opcode (without operand)
			 * [3] result id
			 * [4] result type
			 * * */
			XORSWAP(rdwr_lst_scale_buf[2], rdwr_lst_scale_buf[4]);

			/** we have four operands (including opcode) */
			rdwr_lst_scale_buf[2] |= aclspv_mk_noprnds(4);

			/**
			 * swap [3] and [4].
			 *
			 * after this will
			 * [2] opcode
			 * [3] result type
			 * [4] result id
			 *
			 * that matches what we want.
			 * */
			XORSWAP(rdwr_lst_scale_buf[3], rdwr_lst_scale_buf[4]);
#undef	XORSWAP
			ae2f_fallthrough;
		case ACLUTIL_LITERAL_MASK_CONSTANT:

#define		TMPL_SECTION	(*((rdwr_prv_extra->m_mask_literal) == ACLUTIL_LITERAL_RT \
			? &(rdwr_data)->m_ctx->m_section.m_fnimpl		\
			: &(rdwr_data)->m_ctx->m_section.m_types))

#define		TMPL_COUNT	(*((rdwr_prv_extra->m_mask_literal) == ACLUTIL_LITERAL_RT	\
			? &(rdwr_data)->m_ctx->m_count.m_fnimpl			\
			: &(rdwr_data)->m_ctx->m_count.m_types))

			dbg_puts("[carve2::arithmetic] vec_with_copy");

			ae2f_ifnezerr_strict(s_aclmem_cgrow2(
					&TMPL_SECTION
					, TMPL_COUNT + 6u - (aclspv_wrd_t)(rdwr_prv_extra->m_mask_literal == (aclspv_wrd_t)ACLUTIL_LITERAL_RT)
					, sizeof(aclspv_wrd_t)
					, ACLSPV_COMPILE_ALLOC_FAILED_EXPR
				       )) return c_lbl_err;

			{
				aclspv_wrd_t	CI = 6 - (!rdwr_prv_extra->m_mask_literal);
				while(CI --> 0) {
					aclutil_get_wrd_of_vec(&TMPL_SECTION)[TMPL_COUNT + CI]
						= rdwr_lst_scale_buf[1u + (!rdwr_prv_extra->m_mask_literal) + CI];
				}
			}

			TMPL_COUNT += 6u - (aclspv_wrd_t)(rdwr_prv_extra->m_mask_literal == ACLUTIL_LITERAL_RT);
			break;
		default:
			err_puts(("unknown literal"));
			ae2f_unreachable();
	}

	if(ISASS) {
		CXCursor	CASSIGN		= rdwr_lst_extra->m_cursor;
		aclspv_wrd_t IDX = aclutil_find_cursor(
				rdwr_data->m_ctx->m_num_cursor
				, rdwr_data->m_ctx->m_cursors.m_p
				, clang_getCursorReferenced(CASSIGN)
				);

		aclspv_wrd_t	CLVAL = aclcursor_vardecl_assign(
				&((x_aclcursor* ae2f_restrict)rdwr_data->m_ctx->m_cursors.m_p)[IDX]
				, rdwr_data->m_fnctx, *rdwr_prv_idval, rdwr_prv_extra->m_type_original
				, rdwr_prv_extra->m_mask_literal
				, &rdwr_prv_extra->m_literal
				, 0
				);

		ae2f_ifezerr_strict(CLVAL) {
			err_puts("[carve] aclutil_assign_cursor_lval has been failed.");
			rdwr_data->m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
			return c_lbl_err;
		}
	}

	return c_lbl_success;
}}
