#include "./arithmetic.h"
#include <aclexpr/carve/util.h>
#include <util/wrdemit.h>
#include <.aclspv/vec.h>
#include <ae2f/c90/StdInt/enum.h>
#include <ae2f/cc/switchctl.h>

enum S_ARITHMETIC_ {
	S_ARITHMETIC_ADD	= ACLEXPR_OP2_ARITHMETIC_ADD,
	S_ARITHMETIC_SUB	= ACLEXPR_OP2_ARITHMETIC_SUB,
	S_ARITHMETIC_MUL	= ACLEXPR_OP2_ARITHMETIC_MUL,
	S_ARITHMETIC_DIV	= ACLEXPR_OP2_ARITHMETIC_DIV,
	S_ARITHMETIC_REM	= ACLEXPR_OP2_ARITHMETIC_REM
};

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
			if(aclid_is_int(ID))
				return SpvOpIAdd;
			else if(aclid_is_float(ID))
				return SpvOpFAdd;
			else return 0;

		case S_ARITHMETIC_DIV:
			if(aclid_is_signed(ID))
				return SpvOpSDiv;
			else if (aclid_is_unsigned(ID))
				return SpvOpUDiv;
			else if(aclid_is_float(ID))
				return SpvOpFDiv;
			else return 0;

		case S_ARITHMETIC_MUL:
			if(aclid_is_signed(ID))
				return SpvOpIMul;
			else if (aclid_is_unsigned(ID))
				return SpvOpIMul;
			else if(aclid_is_float(ID))
				return SpvOpFMul;
			else return 0;

		case S_ARITHMETIC_REM:
			if(aclid_is_signed(ID))
				return SpvOpSRem;
			else if (aclid_is_unsigned(ID))
				return SpvOpUMod;
			else if(aclid_is_float(ID))
				return SpvOpFMod;
			else return 0;

		case S_ARITHMETIC_SUB:
			if(aclid_is_signed(ID))
				return SpvOpISub;
			else if (aclid_is_unsigned(ID))
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

		case ACLEXPR_OP2_ARITHMETIC_ADD:
		case ACLEXPR_OP2_ARITHMETIC_SUB:
		case ACLEXPR_OP2_ARITHMETIC_DIV:
		case ACLEXPR_OP2_ARITHMETIC_MUL:
		case ACLEXPR_OP2_ARITHMETIC_REM:
		case ACLEXPR_OP2_ASSIGN:
			return 1;
	}
}

#define CHECK unless(	\
		s_checkhelper(	\
				rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]	\
				)) return c_lbl_continue

ae2f_inline_f static ae2f_ccconst struct aclutil_ltrl_eval_t	aclutil_literal_eval2(
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

aclexpr_carve_lbl_t aclexpr_carve_op2_arithmetic(
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
	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL] = ACLUTIL_LITERAL_MASK_LITERAL;
	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]	= *rdwr_prv_idval = CTX(rdwr_data)->m_id++;

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] = aclid_decide_cast(
			(enum ACLSPV_ID_) rdwr_lst_extra[0].m_type_original
			, (enum ACLSPV_ID_) rdwr_lst_extra[1].m_type_original
			, CTX(rdwr_data)
			);	/** only when bin2 (arithmetic thing) */

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];

	if(aclid_is_float(aclutil_cxtp_get_veccfg(rdwr_prv_extra->m_type_original, rdwr_data->m_ctx).m_ty_elm)) {
		/** FIXME: make this interchangable by compatibility */
		rdwr_prv_extra->m_mask_literal &= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL] &= ~(aclspv_wrd_t)ACLUTIL_LITERAL_CONSTANT;
	}

	ae2fpat_assertassume_strict(rdwr_lst_scale_buf);
	ae2fpat_assertassume_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]);

#define	OPITER(NUM_OPRND) \
	{										\
		if(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] != rdwr_lst_extra[NUM_OPRND].m_type_original) {	\
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
														return c_lbl_err;							\
				\
				case ACLEXPR_RET_SUCCESS:								\
															rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL] &= 0;			\
				ae2f_fallthrough;							\
				case ACLEXPR_RET_SUCCESS_CONSTANT:						\
														rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL] &= 1;			\
				ae2f_fallthrough;							\
				case ACLEXPR_RET_SUCCESS_LITERAL:							\
															break;									\
			}											\
		}	\
		\
		rdwr_prv_extra->m_mask_literal									\
		&= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL]					\
		&= rdwr_lst_extra[NUM_OPRND].m_mask_literal;						\
		\
		dbg_prefix(i);											\
		dbg_call((void), fprintf(stderr, "deepbin2-%u[%u]{%u} and literal mask %p [%u]{%u}\n"		\
					, NUM_OPRND, *rdwr_prv_idval, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]	\
					, (void*)&rdwr_lst_extra[NUM_OPRND].m_mask_literal				\
					, rdwr_lst_extra[NUM_OPRND].m_mask_literal					\
					, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL]					\
					));										\
	}

	OPITER(1u);
	OPITER(0u);

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] = s_op_to_spv(
				ACLEXPR_OPBINCFG_RESTYID[rdwr_lst_scale_buf]
				, (ACLEXPR_OPBINCFG_OPCODE[rdwr_lst_scale_buf])
				, CTX(rdwr_data)
				)) {
		err_puts(("SpvOp went undefined"));
		return c_lbl_err;
	} else {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", rdwr_lst_scale_buf[5])));
	}

	switch(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_MASKLITERAL]) {
		case ACLUTIL_LITERAL_MASK_RT:
#define	XORSWAP(a, b)	{ (a) ^= (b); (b) ^= (a); (a) ^= (b); }
			/** 
			 * here, operation is not constant.
			 *
			 * current
			 * [2] OpSpecConstant opcode (ignorance required)
			 * [3] result type
			 * [4] result id
			 * [5] literal opcode (opspecconstant evaluation)
			 * ...
			 *
			 *
			 * we want
			 * [3] opcode with operand count
			 * [4] result type
			 * [5] result id
			 * */

			/** 
			 * swap [3] and [5]
			 *
			 * after this will
			 * [3] literal opcode (without operand)
			 * [4] result id
			 * [5] result type
			 * * */
			XORSWAP(rdwr_lst_scale_buf[3], rdwr_lst_scale_buf[5]);

			/** we have four operands (including opcode) */
			rdwr_lst_scale_buf[3] |= mk_noprnds(4);

			/**
			 * swap [4] and [5].
			 *
			 * after this will
			 * [3] opcode
			 * [4] result type
			 * [5] result id
			 *
			 * that matches what we want.
			 * */
			XORSWAP(rdwr_lst_scale_buf[4], rdwr_lst_scale_buf[5]);
#undef	XORSWAP
			break;

		case ACLUTIL_LITERAL_MASK_LITERAL: 
		case ACLUTIL_LITERAL_LITERAL:
			{
				struct aclutil_ltrl_eval_t EVAL_LIT 
					= aclutil_literal_eval2(
							rdwr_lst_extra[0].m_literal
							, rdwr_lst_extra[1].m_literal
							, (enum SpvOp_)rdwr_lst_scale_buf[5]
							,(enum ACLSPV_ID_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
							);

				if(EVAL_LIT.m_found) {
					dbg_puts(("The result is literal."));
					*r_ltrl = rdwr_prv_extra->m_literal = EVAL_LIT.m_ret;
					rdwr_prv_extra->m_mask_literal = 3;
					rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
					rdwr_lst_scale_buf[1] = 3;
					switch(aclexpr_arithmetic_cast_literal(
								rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
								, EVAL_LIT.m_ret
								, rdwr_prv_idval
								, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
								, 0, CTX(rdwr_data))) {
						case ACLEXPR_RET_SUCCESS:
						case ACLEXPR_RET_SUCCESS_CONSTANT:
						default:
							ae2f_unreachable();

						case ACLEXPR_RET_NOT_THE_CASE:
						case ACLEXPR_RET_FAILURE:
							return c_lbl_err;

						case ACLEXPR_RET_SUCCESS_LITERAL:
							break;
					}

					dbg_prefix();
					dbg_call((void), fprintf(stderr, "literal id: %u\n", *rdwr_prv_idval));
					return 0;
				}
			}

			ae2f_fallthrough;
		case ACLUTIL_LITERAL_MASK_CONSTANT: 
			dbg_puts(("The result is constant(fallback)."));
			rdwr_lst_scale_buf[1] = ACLUTIL_LITERAL_MASK_CONSTANT;
			break;
		default:
			err_puts(("unknown literal"));
			ae2f_unreachable();
	}

#define		TMPL_SECTION	(*((rdwr_lst_scale_buf[1]) == ACLUTIL_LITERAL_RT \
			? &CTX(rdwr_data)->m_section.m_fnimpl		\
			: &CTX(rdwr_data)->m_section.m_types))

#define		TMPL_COUNT	(*((rdwr_lst_scale_buf[1]) == ACLUTIL_LITERAL_RT	\
			? &CTX(rdwr_data)->m_count.m_fnimpl			\
			: &CTX(rdwr_data)->m_count.m_types))

	dbg_puts("vec_with_copy");
	_aclspv_grow_vec_with_copy(
			_aclspv_malloc
			, _aclspv_free
			, _aclspv_memcpy
			, L_new
			, TMPL_SECTION
			, (size_t)aclutil_count_to_sz(TMPL_COUNT + (rdwr_lst_scale_buf[0] + ACLEXPR_OPBINCFG_RESID)
				- (rdwr_lst_scale_buf[1] == ACLUTIL_LITERAL_RT))
			);

	ae2f_ifezerr(TMPL_SECTION.m_p) {
		CTX(rdwr_data)->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
		return c_lbl_err;
	}

	dbg_puts("TMPCHECK: Is before memcpy okay?");

#define        LST_CMD_COUNT           (ACLEXPR_OPBINCFG_RESID /** header */ + (rdwr_lst_scale_buf[0]))

	{
		aclspv_wrd_t	CI = LST_CMD_COUNT - (!rdwr_lst_scale_buf[1]);
		while(CI --> 0) {
			aclutil_get_wrd_of_vec(&TMPL_SECTION)[TMPL_COUNT + CI] 
				= rdwr_lst_scale_buf[(aclspv_wrd_t)ACLEXPR_OPBINCFG_SPECCONSTANTOP + (!rdwr_lst_scale_buf[1]) + CI];
		}
	}

	TMPL_COUNT += LST_CMD_COUNT - (rdwr_lst_scale_buf[1] == ACLUTIL_LITERAL_RT);
	return c_lbl_success;
}}
