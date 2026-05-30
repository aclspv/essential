#include "./arithmetic.h"
#include <aclexpr/carve/util.h>
#include <util/wrdemit.h>
#include <.aclspv/vec.h>

static ae2f_ccconst aclspv_bool_t s_checkhelper(const aclspv_wrd_t x) {
	switch(x) {
		default:
			return 0;

		case UTIL_OPBIN2_ADD:
		case UTIL_OPBIN2_SUB:
		case UTIL_OPBIN2_DIV:
		case UTIL_OPBIN2_MUL:
		case UTIL_OPBIN2_REM:
		case UTIL_OPBIN2_ASSIGN:
			return 1;
	}
}

#define CHECK unless(	\
		s_checkhelper(	\
				rdwr_lst_scale_buf[UTIL_OPBINCFG_OPCODE]	\
				)) return c_lbl_continue

aclexpr_carve_lbl_t aclexpr_carve_op2_arithmetic(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		util_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		util_literal* ae2f_restrict			r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		) { CHECK; {
	rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] = UTIL_LITERAL_MASK_LITERAL;
	rdwr_lst_scale_buf[UTIL_OPBINCFG_RESID]	= *rdwr_prv_idval = CTX(rdwr_data)->m_id++;

	rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID] = aclid_decide_cast(
			(e_id_default) rdwr_lst_extra[0].m_type_original
			, (e_id_default) rdwr_lst_extra[1].m_type_original
			, CTX(rdwr_data)
			);	/** only when bin2 (arithmetic thing) */

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID];

	if(aclid_is_float(aclutil_cxtp_get_veccfg(rdwr_prv_extra->m_type_original, rdwr_data->m_ctx).m_ty_elm)) {
		/** FIXME: make this interchangable by compatibility */
		rdwr_prv_extra->m_mask_literal &= rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] &= ~(aclspv_wrd_t)UTIL_LITERAL_CONSTANT;
	}

	ae2fpat_assertassume_strict(rdwr_lst_scale_buf);
	ae2fpat_assertassume_strict(rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]);

#define	OPITER(NUM_OPRND) \
	{										\
		if(rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID] != rdwr_lst_extra[NUM_OPRND].m_type_original) {	\
			const util_literal OLD_LITERAL = rdwr_lst_extra[NUM_OPRND].m_literal;			\
			aclspv_id_t OLD_ID 									\
			= rdwr_lst_scale_buf[UTIL_OPBINCFG_END + NUM_OPRND];					\
			rdwr_lst_scale_buf[UTIL_OPBINCFG_END + NUM_OPRND] = CTX(rdwr_data)->m_id++;		\
			\
			dbg_prefix(a);										\
			dbg_call((void), fprintf(stderr, "ARITHMETIC CAST ON LOOP[%u]: %u %u\n"			\
						, NUM_OPRND							\
						, OLD_ID, rdwr_lst_scale_buf[UTIL_OPBINCFG_END + NUM_OPRND]	\
						));								\
			\
			switch(aclexpr_arithmetic_cast(								\
						OLD_ID								\
						, rdwr_lst_extra[NUM_OPRND].m_type_original			\
						, rdwr_lst_extra[NUM_OPRND].m_mask_literal			\
						, OLD_LITERAL							\
						, &rdwr_lst_scale_buf[UTIL_OPBINCFG_END + NUM_OPRND]		\
						, rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]			\
						, &rdwr_lst_extra[NUM_OPRND].m_literal				\
						, CTX(rdwr_data)						\
						))								\
			{											\
				default:									\
				case EMIT_EXPR_FAILURE:								\
				case EMIT_EXPR_NOT_THE_CASE:							\
														return c_lbl_err;							\
				\
				case EMIT_EXPR_SUCCESS:								\
														rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] &= 0;			\
				ae2f_fallthrough;							\
				case EMIT_EXPR_SUCCESS_CONSTANT:						\
														rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] &= 1;			\
				ae2f_fallthrough;							\
				case EMIT_EXPR_SUCCESS_LITERAL:							\
														break;									\
			}											\
		}	\
		\
		rdwr_prv_extra->m_mask_literal									\
		&= rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL]					\
		&= rdwr_lst_extra[NUM_OPRND].m_mask_literal;						\
		\
		dbg_prefix(i);											\
		dbg_call((void), fprintf(stderr, "deepbin2-%u[%u]{%u} and literal mask %p [%u]{%u}\n"		\
					, NUM_OPRND, *rdwr_prv_idval, rdwr_lst_scale_buf[UTIL_OPBINCFG_END + NUM_OPRND]	\
					, (void*)&rdwr_lst_extra[NUM_OPRND].m_mask_literal				\
					, rdwr_lst_extra[NUM_OPRND].m_mask_literal					\
					, rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL]					\
					));										\
	}

	OPITER(1u);
	OPITER(0u);

	ae2f_ifezerr(rdwr_lst_scale_buf[UTIL_OPBINCFG_OPCODE] = util_opbin2_to_spvop(
				(e_id_default)UTIL_OPBINCFG_RESTYID[rdwr_lst_scale_buf]
				, (enum UTIL_OPBIN2_)UTIL_OPBINCFG_OPCODE[rdwr_lst_scale_buf]
				, CTX(rdwr_data)
				)) {
		err_puts(("SpvOp went undefined"));
		return c_lbl_err;
	} else {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", rdwr_lst_scale_buf[5])));
	}

	switch(rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL]) {
		case UTIL_LITERAL_MASK_RT:
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

		case UTIL_LITERAL_MASK_LITERAL: 
		case UTIL_LITERAL_LITERAL:
			{
				struct util_literal_eval_ EVAL_LIT 
					= util_literal_eval2(
							rdwr_lst_extra[0].m_literal
							, rdwr_lst_extra[1].m_literal
							, (enum SpvOp_)rdwr_lst_scale_buf[5]
							,(e_id_default)rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]
							);

				if(EVAL_LIT.m_found) {
					dbg_puts(("The result is literal."));
					*r_ltrl = rdwr_prv_extra->m_literal = EVAL_LIT.m_ret;
					rdwr_prv_extra->m_mask_literal = 3;
					rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID];
					rdwr_lst_scale_buf[1] = 3;
					switch(aclexpr_arithmetic_cast_literal(
								rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]
								, EVAL_LIT.m_ret
								, rdwr_prv_idval
								, rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]
								, 0, CTX(rdwr_data))) {
						case EMIT_EXPR_SUCCESS:
						case EMIT_EXPR_SUCCESS_CONSTANT:
						default:
							ae2f_unreachable();

						case EMIT_EXPR_NOT_THE_CASE:
						case EMIT_EXPR_FAILURE:
							return c_lbl_err;

						case EMIT_EXPR_SUCCESS_LITERAL:
							break;
					}

					dbg_prefix();
					dbg_call((void), fprintf(stderr, "literal id: %u\n", *rdwr_prv_idval));
					return 0;
				}
			}

			ae2f_fallthrough;
		case UTIL_LITERAL_MASK_CONSTANT: 
			dbg_puts(("The result is constant(fallback)."));
			rdwr_lst_scale_buf[1] = UTIL_LITERAL_MASK_CONSTANT;
			break;
		default:
			err_puts(("unknown literal"));
			ae2f_unreachable();
	}

#define		TMPL_SECTION	(*((rdwr_lst_scale_buf[1]) == UTIL_LITERAL_RT \
			? &CTX(rdwr_data)->m_section.m_fnimpl		\
			: &CTX(rdwr_data)->m_section.m_types))

#define		TMPL_COUNT	(*((rdwr_lst_scale_buf[1]) == UTIL_LITERAL_RT	\
			? &CTX(rdwr_data)->m_count.m_fnimpl			\
			: &CTX(rdwr_data)->m_count.m_types))

	dbg_puts("vec_with_copy");
	_aclspv_grow_vec_with_copy(
			_aclspv_malloc
			, _aclspv_free
			, _aclspv_memcpy
			, L_new
			, TMPL_SECTION
			, (size_t)count_to_sz(TMPL_COUNT + (rdwr_lst_scale_buf[0] + UTIL_OPBINCFG_RESID)
				- (rdwr_lst_scale_buf[1] == UTIL_LITERAL_RT))
			);

	ae2f_ifezerr(TMPL_SECTION.m_p) {
		CTX(rdwr_data)->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
		return c_lbl_err;
	}

	dbg_puts("TMPCHECK: Is before memcpy okay?");

#define        LST_CMD_COUNT           (UTIL_OPBINCFG_RESID /** header */ + (rdwr_lst_scale_buf[0]))

	{
		aclspv_wrd_t	CI = LST_CMD_COUNT - (!rdwr_lst_scale_buf[1]);
		while(CI --> 0) {
			get_wrd_of_vec(&TMPL_SECTION)[TMPL_COUNT + CI] 
				= rdwr_lst_scale_buf[(aclspv_wrd_t)UTIL_OPBINCFG_SPECCONSTANTOP + (!rdwr_lst_scale_buf[1]) + CI];
		}
	}

	TMPL_COUNT += LST_CMD_COUNT - (rdwr_lst_scale_buf[1] == UTIL_LITERAL_RT);
	return c_lbl_success;
}}
