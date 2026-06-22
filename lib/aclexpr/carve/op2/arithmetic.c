#include "./arithmetic.h"

#include <aclexpr/carve/util.h>
#include <aclcursor.h>

#include <util/wrdemit.h>
#include <.aclspv/vec.h>
#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/dbg.h>
#include <util/constant.h>

enum S_ARITHMETIC_ {
	S_ARITHMETIC_ADD	= ACLEXPR_OP2_ADD,
	S_ARITHMETIC_ADDX	= ACLEXPR_OP2_ADDASS,
	S_ARITHMETIC_SUB	= ACLEXPR_OP2_SUB,
	S_ARITHMETIC_SUBX	= ACLEXPR_OP2_SUBASS,
	S_ARITHMETIC_MUL	= ACLEXPR_OP2_MUL,
	S_ARITHMETIC_MULX	= ACLEXPR_OP2_MULASS,
	S_ARITHMETIC_DIV	= ACLEXPR_OP2_DIV,
	S_ARITHMETIC_DIVX	= ACLEXPR_OP2_DIVASS,
	S_ARITHMETIC_REM	= ACLEXPR_OP2_REM,
	S_ARITHMETIC_REMX	= ACLEXPR_OP2_REMASS
};

ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_is_assign(aclspv_wrd_t c_opcode) {

	switch(c_opcode) {
		case S_ARITHMETIC_ADD:
		case S_ARITHMETIC_DIV:
		case S_ARITHMETIC_MUL:
		case S_ARITHMETIC_REM:
		case S_ARITHMETIC_SUB:
		default:
			return 0;

		case S_ARITHMETIC_ADDX:
		case S_ARITHMETIC_DIVX:
		case S_ARITHMETIC_MULX:
		case S_ARITHMETIC_SUBX:
		case S_ARITHMETIC_REMX:
			return 1;
	}
}

ae2f_inline_f static ae2f_ccpure aclspv_opcode_t s_op_to_spv(
		const aclspv_id_t c_id,
		const aclspv_wrd_t c_op2,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t	ID = aclid_is_vectorisable(c_id) ? c_id : aclutil_cxtp_get_veccfg(c_id, h_ctx).m_ty_elm;

	switch((enum S_ARITHMETIC_)c_op2) {
		default:
			return 0;

		case S_ARITHMETIC_ADD:
		case S_ARITHMETIC_ADDX:
			if(aclid_is_int(ID))
				return SpvOpIAdd;
			else if(aclid_is_float(ID))
				return SpvOpFAdd;
			else return 0;

		case S_ARITHMETIC_DIV:
		case S_ARITHMETIC_DIVX:
			if(aclid_is_sint(ID))
				return SpvOpSDiv;
			else if (aclid_is_uint(ID))
				return SpvOpUDiv;
			else if(aclid_is_float(ID))
				return SpvOpFDiv;
			else return 0;

		case S_ARITHMETIC_MUL:
		case S_ARITHMETIC_MULX:
			if(aclid_is_sint(ID))
				return SpvOpIMul;
			else if (aclid_is_uint(ID))
				return SpvOpIMul;
			else if(aclid_is_float(ID))
				return SpvOpFMul;
			else return 0;

		case S_ARITHMETIC_REM:
		case S_ARITHMETIC_REMX:
			if(aclid_is_sint(ID))
				return SpvOpSRem;
			else if (aclid_is_uint(ID))
				return SpvOpUMod;
			else if(aclid_is_float(ID))
				return SpvOpFMod;
			else return 0;

		case S_ARITHMETIC_SUB:
		case S_ARITHMETIC_SUBX:
			if(aclid_is_sint(ID))
				return SpvOpISub;
			else if (aclid_is_uint(ID))
				return SpvOpISub;
			else if(aclid_is_float(ID))
				return SpvOpFSub;
			else return 0;
	}
}

ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_checkhelper(const aclspv_wrd_t x) {
	switch(x) {
		default:
			return 0;

		case ACLEXPR_OP2_ADD:
		case ACLEXPR_OP2_SUB:
		case ACLEXPR_OP2_DIV:
		case ACLEXPR_OP2_MUL:
		case ACLEXPR_OP2_REM:
		case ACLEXPR_OP2_ADDASS:
		case ACLEXPR_OP2_SUBASS:
		case ACLEXPR_OP2_DIVASS:
		case ACLEXPR_OP2_MULASS:
		case ACLEXPR_OP2_REMASS:
			return 1;
	}
}

#define CHECK unless(	\
		s_checkhelper(	\
				rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]	\
				)) return c_lbl_continue

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
				EXPR_TWO(+, m_api_intmax, SpvOpIAdd); return RET;
				EXPR_TWO(-, m_api_intmax, SpvOpISub); return RET;
				EXPR_TWO(*, m_api_intmax, SpvOpIMul); return RET;
				EXPR_TWO(/, m_api_intmax, SpvOpSDiv); return RET;
				EXPR_TWO(/, m_api_intmax, SpvOpUDiv); return RET;
				EXPR_TWO(%, m_api_intmax, SpvOpSRem); return RET;

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
				EXPR_TWO(+, m_api_uintmax, SpvOpIAdd); return RET;
				EXPR_TWO(-, m_api_uintmax, SpvOpISub); return RET;
				EXPR_TWO(*, m_api_uintmax, SpvOpIMul); return RET;
				EXPR_TWO(/, m_api_uintmax, SpvOpSDiv); return RET;
				EXPR_TWO(/, m_api_uintmax, SpvOpUDiv); return RET;
				EXPR_TWO(%, m_api_uintmax, SpvOpSRem); return RET;

				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		case ACLSPV_ID_F32:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(+, m_flt, SpvOpFAdd); return RET;
				EXPR_TWO(-, m_flt, SpvOpFSub); return RET;
				EXPR_TWO(/, m_flt, SpvOpFDiv); return RET;
				EXPR_TWO(*, m_flt, SpvOpFMul); return RET;

				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		case ACLSPV_ID_F64:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(+, m_dbl, SpvOpFAdd); return RET;
				EXPR_TWO(-, m_dbl, SpvOpFSub); return RET;
				EXPR_TWO(/, m_dbl, SpvOpFDiv); return RET;
				EXPR_TWO(*, m_dbl, SpvOpFMul); return RET;
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

static aclspv_wrd_t s_decide_cast(const enum ACLSPV_ID_ c_id0, const enum ACLSPV_ID_ c_id1, const h_aclspv_ctx_t h_ctx) {
	/** one is 0? */
	unless(c_id0 && c_id1)
		return ae2f_bll_sel_bool_naive1(B_id_default_t, c_id0, (B_id_default_t) c_id0, (B_id_default_t) c_id1);

	unless(c_id0 ^ c_id1)
		return (B_id_default_t) c_id0;

	unless(aclid_is_arithmetic((B_id_default_t) c_id0, h_ctx) & aclid_is_arithmetic((B_id_default_t) c_id1, h_ctx))
		return 0;

	if(aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num ^ aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num) {
		unless(aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id1;
		} else unless(aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id0;
		} return 0;
	}

	unless(aclid_is_vectorisable((B_id_default_t) c_id0)) {
		return ae2f_bll_sel_bool1(B_id_default_t, s_decide_cast(
				(enum ACLSPV_ID_) aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_ty_elm
					, (enum ACLSPV_ID_) aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_ty_elm
					, h_ctx
					), (B_id_default_t) c_id0, (B_id_default_t) c_id1);
	}

	if(aclid_is_float((B_id_default_t) c_id0) != aclid_is_float((B_id_default_t) c_id1))
		return ae2f_bll_sel_bool1(B_id_default_t, aclid_is_float((B_id_default_t) c_id0), (B_id_default_t) c_id0, (B_id_default_t)c_id1);

	if(aclid_is_int((aclspv_wrd_t) c_id0) && aclid_is_int((aclspv_wrd_t)c_id1) && aclid_is_sint((aclspv_wrd_t)c_id0) != aclid_is_sint((aclspv_wrd_t)c_id1))
		return ae2f_bll_sel_bool1(aclspv_wrd_t, aclid_is_sint((aclspv_wrd_t)c_id0), (aclspv_wrd_t)c_id0, (aclspv_wrd_t)c_id1);

	return ae2f_bll_sel_bool1(
			B_id_default_t
			, aclid_get_bit_width((B_id_default_t) c_id0) > aclid_get_bit_width((B_id_default_t) c_id1)
			, (B_id_default_t) c_id0 , (B_id_default_t) c_id1);
}

aclexpr_carve_lbl_t aclexpr_carve2_arithmetic(
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
		) { CHECK; {
	const aclspv_bool_t IS_ASSIGN = s_is_assign(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]);

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] = s_decide_cast(
			(enum ACLSPV_ID_) rdwr_lst_extra[0].m_type_original
			, (enum ACLSPV_ID_) rdwr_lst_extra[1].m_type_original
			, CTX(rdwr_data)
			);

	rdwr_prv_extra->m_mask_literal = rdwr_lst_extra[0].m_mask_literal & rdwr_lst_extra[1].m_mask_literal;

	ae2fpat_assertassume_strict(rdwr_lst_scale_buf);

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]) {
		err_puts("[carve2::arithmetic] no result type");
		err_prefix(0);
		err_call((void), fprintf(stderr, "[carve2::arithmetic] opcode: %u %u\n"
					, (unsigned)rdwr_lst_extra[0].m_type_original
					, (unsigned)rdwr_lst_extra[1].m_type_original));
		return c_lbl_err;
	}

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]	= *rdwr_prv_idval = CTX(rdwr_data)->m_id++;

#define	OPITER(NUM_OPRND) \
	{										\
		{	\
			const aclutil_literal OLD_LITERAL = rdwr_lst_extra[NUM_OPRND].m_literal;			\
			aclspv_id_t OLD_ID 									\
			= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND];					\
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND] = CTX(rdwr_data)->m_id++;		\
			\
			dbg_prefix(a);										\
			dbg_call((void), fprintf(stderr, "ARITHMETIC CAST ON LOOP[%u]: %u %u\n"			\
						, NUM_OPRND							\
						, OLD_ID, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]	\
						));								\
			\
			switch(aclexpr_arithmetic_cast(								\
						OLD_ID								\
						, rdwr_lst_extra[NUM_OPRND].m_type_original			\
						, rdwr_lst_extra[NUM_OPRND].m_mask_literal			\
						, OLD_LITERAL							\
						, &rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]		\
						, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]			\
						, &rdwr_lst_extra[NUM_OPRND].m_literal				\
						, CTX(rdwr_data)						\
						))								\
			{											\
				default:									\
				case ACLEXPR_RET_FAILURE:								\
				case ACLEXPR_RET_NOT_THE_CASE:							\
														err_puts("[carve2::arithmetic_cast] OPITER"); return c_lbl_err;							\
				\
				case ACLEXPR_RET_SUCCESS:								\
															rdwr_prv_extra->m_mask_literal &= 0;			\
				ae2f_fallthrough;							\
				case ACLEXPR_RET_SUCCESS_CONSTANT:						\
														rdwr_prv_extra->m_mask_literal &= 1;			\
				ae2f_fallthrough;							\
				case ACLEXPR_RET_SUCCESS_LITERAL:							\
															break;									\
			}											\
		}	\
		\
		dbg_prefix(i);											\
		dbg_call((void), fprintf(stderr, "deepbin2-%u[%u]{%u} and literal mask %p [%u]{%u}\n"		\
					, NUM_OPRND	\
					, *rdwr_prv_idval, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]	\
					, (void*)&rdwr_lst_extra[NUM_OPRND].m_mask_literal				\
					, rdwr_lst_extra[NUM_OPRND].m_mask_literal					\
					, rdwr_prv_extra->m_mask_literal					\
					));										\
	}

	OPITER(1u);
	OPITER(0u);

	if(
			aclid_is_float(aclutil_cxtp_get_veccfg(rdwr_prv_extra[0].m_type_original, rdwr_data->m_ctx).m_ty_elm)
			|| aclid_is_float(aclutil_cxtp_get_veccfg(rdwr_prv_extra[1].m_type_original, rdwr_data->m_ctx).m_ty_elm)
			) {
		fixme_puts("this is float. make sure we can do it constantly with Kernel capability.");
		rdwr_prv_extra->m_mask_literal &= ~(aclspv_wrd_t)ACLUTIL_LITERAL_CONSTANT;
	}

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] = s_op_to_spv(
				ACLEXPR_OPBINCFG_RESTYID[rdwr_lst_scale_buf]
				, (ACLEXPR_OPBINCFG_OPCODE[rdwr_lst_scale_buf])
				, CTX(rdwr_data)
				)) {
		err_puts(("[carve2::arithmetic] SpvOp went undefined"));
		return c_lbl_err;
	} else {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])));
	}

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END] >= rdwr_data->m_ctx->m_id)
		return c_lbl_err;

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1] >= rdwr_data->m_ctx->m_id)
		return c_lbl_err;

	switch(rdwr_prv_extra->m_mask_literal) {
		case ACLUTIL_LITERAL_MASK_LITERAL:
		case ACLUTIL_LITERAL_LITERAL:
			{
				struct aclutil_ltrl_eval_t EVAL_LIT
					= s_eval2_ltrl(
							rdwr_lst_extra[0].m_literal
							, rdwr_lst_extra[1].m_literal
							, (enum SpvOp_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]
							,(enum ACLSPV_ID_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
						      );

				ae2f_ifnezerr_strict(EVAL_LIT.m_found) {
					dbg_puts(("The result is literal."));
					*r_ltrl = rdwr_prv_extra->m_literal = EVAL_LIT.m_ret;
					rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];


					ae2f_ifezerr(*rdwr_prv_idval = aclutil_mk_constant_val_by_enum(EVAL_LIT.m_ret.m_api_uintmax, rdwr_data->m_ctx
								, (int_leastenum_t)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID])) {
						err_puts("[carve2::arithmetic] aclutil_mk_constant_val_by_enum");
						return c_lbl_err;
					}


					dbg_prefix();
					dbg_call((void), fprintf(stderr, "literal id: %u\n", *rdwr_prv_idval));
					break;
				} else {
					err_puts("[carve2::arithmetic] EVAL_LIT");
					return c_lbl_err;
				}
			}

			break;

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
			rdwr_lst_scale_buf[2] |= mk_noprnds(4);

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

			ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END] >= rdwr_data->m_ctx->m_id)
				return c_lbl_err;

			ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1] >= rdwr_data->m_ctx->m_id)
				return c_lbl_err;

			dbg_prefix();
			dbg_call((void), fprintf(stderr
						, "[carve2::arithmetic] check two id: %u %u\n"
						, (unsigned)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
						, (unsigned)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
					  ));

			dbg_puts("[carve2::arithmetic] vec_with_copy");
			_aclspv_grow_vec_with_copy(
					_aclspv_malloc
					, _aclspv_free
					, _aclspv_memcpy
					, L_new
					, TMPL_SECTION
					, (size_t)aclutil_count_to_sz(TMPL_COUNT + 6u - (aclspv_wrd_t)(rdwr_prv_extra->m_mask_literal == (aclspv_wrd_t)ACLUTIL_LITERAL_RT))
					);

			ae2f_ifezerr(TMPL_SECTION.m_p) {
				CTX(rdwr_data)->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
				return c_lbl_err;
			}

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

	if(IS_ASSIGN) {
		CXCursor	CASSIGN		= rdwr_lst_extra->m_cursor;
		aclspv_wrd_t IDX = aclutil_find_cursor(
				rdwr_data->m_ctx->m_num_cursor
				, rdwr_data->m_ctx->m_cursors.m_p
				, (clang_getCursorReferenced(CASSIGN))
				);

		x_aclcursor* ae2f_restrict PC = &((x_aclcursor* ae2f_restrict)rdwr_data->m_ctx->m_cursors.m_p)[IDX];

		aclspv_wrd_t	CLVAL;

		dbg_puts("[carve2::arithmetic] assign mode");

		ae2f_ifnezerr_strict(IDX >= rdwr_data->m_ctx->m_num_cursor) {
			err_puts("[carve2::arithmetic] CIDX assert");
			return c_lbl_err;
		}

		CLVAL = aclcursor_vardecl_assign(
				PC
				, rdwr_data->m_fnctx, *rdwr_prv_idval
				, rdwr_prv_extra->m_type_original
				, rdwr_prv_extra->m_mask_literal
				, &rdwr_prv_extra->m_literal
				, 0
				);

		ae2f_ifezerr(CLVAL) {
			err_puts("[carve] aclutil_assign_cursor_lval has been failed.");
			rdwr_data->m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_VISIT;
			return c_lbl_err;
		}

		dbg_call((void), fprintf(stderr, "[carve2::arithmetic] assigned %u\n", CLVAL));
	}
	return c_lbl_success;
}}
