#include "./carve.h"
#include "util/op.h"
#include <util/wrdemit.h>
#include <util/scale.h>
#include <util/cursor.h>
#include <emit/expr.h>

aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		) {
#define CTX			rdwr_data->m_ctx
#define EXPRSTCK_COUNT		rdwr_data->m_exprcount
#define	NUM_CMDCOUNTERSTCK	rdwr_data->m_num_cmdcounter
#define	RET_LITERAL_REF		rdwr_data->m_ret_literal

#define	EXPRSTCK	CTX->m_tmp.m_v0
#define	CMDSTCK_SCALE	CTX->m_tmp.m_v1
#define	CMDCOUNTERSTCK	CTX->m_tmp.m_v2
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE);
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
	util_literal* ae2f_restrict RET_LITERAL
		= &RET_LITERAL_REF;

	aclspv_wrd_t	NUM_OPRND;

	util_opbincfg_extra* ae2f_restrict LST_EXTRA; 


	typedef struct {
		u32_least	m_id_req;
	} exprstck;

	typedef struct {
		u32_least	m_count_oprnd;
	} cmdcounterstckelm;

	ae2f_unused typedef char aclspv_STATICASSERT_ELMSIZE[
		sizeof(exprstck) == sizeof(cmdcounterstckelm)
			&& sizeof(cmdcounterstckelm) == sizeof(u32_least)
	];

#define	BUF_EXPRSTCK(a)	((exprstck* ae2f_restrict)(EXPRSTCK).m_p)
#define	GETLASTELEXPRSTCK(a)	BUF_EXPRSTCK(c_stck)[EXPRSTCK_COUNT - 1]
#define	BUF_CMDCOUNTERSTCK(a)	((cmdcounterstckelm* ae2f_restrict)(CMDCOUNTERSTCK).m_p)
#define	GETLASTEL_CMDCOUNTERSTCK(a)	BUF_CMDCOUNTERSTCK()[NUM_CMDCOUNTERSTCK - 1]

	NUM_OPRND = LST_SCALE_BUF[0];
	LST_EXTRA
		= (void* ae2f_restrict)(LST_SCALE_BUF + NUM_OPRND + UTIL_OPBINCFG_END);

	ae2fpat_assertassume(LST_SCALE_BUF);
	ae2fpat_assertassume(rdwr_prv_extra);
	ae2fpat_assertassume(rdwr_prv_idval);

	switch(NUM_OPRND) {
		default:
			return 1;
		case 1:
			switch(LST_SCALE_BUF[UTIL_OPBINCFG_OPCODE]) {
				case UTIL_OPBIN1_EXPLICIT_CAST:
					LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL] = LST_EXTRA[0].m_mask_literal;
						*rdwr_prv_idval = LST_SCALE_BUF[UTIL_OPBINCFG_END];
					if(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID] != LST_EXTRA[0].m_type_original) {
						const util_literal OLD_LITERAL = LST_EXTRA[0].m_literal;
						const aclspv_id_t OLD_ID = LST_SCALE_BUF[UTIL_OPBINCFG_END];

						dbg_prefix((Yup, old id is needed));
						dbg_call(fprintf, (stderr, "OLD_ID: %u\n", OLD_ID));

						dbg_prefix((Yup, new type id is needed));
						dbg_call(fprintf, (stderr, "NEW_TYPE: %u\n", LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]));

						switch(aclexpr_arithmetic_cast(
									OLD_ID
									, LST_EXTRA[0].m_type_original
									, LST_EXTRA[0].m_mask_literal
									, OLD_LITERAL
									, rdwr_prv_idval
									, LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]
									, &rdwr_prv_extra->m_literal
									, CTX
									)) {
							default:
							case EMIT_EXPR_NOT_THE_CASE:
							case EMIT_EXPR_FAILURE:
								return 1;

							case EMIT_EXPR_SUCCESS:
								LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL] &= 0;
								ae2f_fallthrough;
							case EMIT_EXPR_SUCCESS_CONSTANT:
								LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL] &= 1;
								ae2f_fallthrough;
							case EMIT_EXPR_SUCCESS_LITERAL:
								break;
						}
					}

					assert(*rdwr_prv_idval);

					rdwr_prv_extra->m_mask_literal
						&= LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL];
					rdwr_prv_extra->m_type_original
						= LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID];
					*RET_LITERAL = rdwr_prv_extra->m_literal;
					return 0;
				default:
					return 1;
			}
		case 2:
			LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL]= UTIL_LITERAL_MASK_LITERAL;
			LST_SCALE_BUF[UTIL_OPBINCFG_RESID]	= *rdwr_prv_idval = CTX->m_id++;

			unless(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID])
				LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID] = aclid_decide_cast(
						(e_id_default) LST_EXTRA[0].m_type_original
						, (e_id_default) LST_EXTRA[1].m_type_original
						, CTX
						);	/** only when bin2 (arithmetic thing) */

			rdwr_prv_extra->m_type_original = LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID];
			if(aclid_is_float(rdwr_prv_extra->m_type_original)) {
				rdwr_prv_extra->m_mask_literal &= ~(aclspv_wrd_t)UTIL_LITERAL_CONSTANT;
			}

			ae2fpat_assertassume_strict(LST_SCALE_BUF);
			ae2fpat_assertassume_strict(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]);

			dbg_prefix(a);
			dbg_call((void), fprintf(stderr, "NUM_OPRND: %u\n", NUM_OPRND));


			/**** @fixme KERNEL SHIT *****/
			unless(aclid_is_int(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID])) {
				rdwr_prv_extra->m_mask_literal
					&= LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL]
					&= UTIL_LITERAL_LITERAL;
			}

			while(NUM_OPRND--) {
				if(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID] != LST_EXTRA[NUM_OPRND].m_type_original) {
					const util_literal OLD_LITERAL = LST_EXTRA[NUM_OPRND].m_literal;
					aclspv_id_t OLD_ID 
						= LST_SCALE_BUF[UTIL_OPBINCFG_END + NUM_OPRND];
					LST_SCALE_BUF[UTIL_OPBINCFG_END + NUM_OPRND] = CTX->m_id++;

					dbg_prefix(a);
					dbg_call((void), fprintf(stderr, "ARITHMETIC CAST ON LOOP[%u]: %u %u\n"
								, NUM_OPRND
								, OLD_ID, LST_SCALE_BUF[UTIL_OPBINCFG_END + NUM_OPRND]
								));

					switch(aclexpr_arithmetic_cast(
								OLD_ID
								, LST_EXTRA[NUM_OPRND].m_type_original
								, LST_EXTRA[NUM_OPRND].m_mask_literal
								, OLD_LITERAL
								, &LST_SCALE_BUF[UTIL_OPBINCFG_END + NUM_OPRND]
								, LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]
								, &LST_EXTRA[NUM_OPRND].m_literal
								, CTX
								))
					{
						default:
						case EMIT_EXPR_FAILURE:
						case EMIT_EXPR_NOT_THE_CASE:
							return 1;

						case EMIT_EXPR_SUCCESS:
							LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL] &= 0;
							ae2f_fallthrough;
						case EMIT_EXPR_SUCCESS_CONSTANT:
							LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL] &= 1;
							ae2f_fallthrough;
						case EMIT_EXPR_SUCCESS_LITERAL:
							break;
					}
				}

				rdwr_prv_extra->m_mask_literal
					&= LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL]
					&= LST_EXTRA[NUM_OPRND].m_mask_literal;

				dbg_prefix(i);
				dbg_call((void), fprintf(stderr, "deepbin2-%u[%u]{%u} and literal mask %p [%u]{%u}\n"
							, NUM_OPRND, *rdwr_prv_idval, LST_SCALE_BUF[UTIL_OPBINCFG_END + NUM_OPRND]
							, (void*)&LST_EXTRA[NUM_OPRND].m_mask_literal
							, LST_EXTRA[NUM_OPRND].m_mask_literal
							, LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL]
							));
			}

			ae2f_expected_but_else(LST_SCALE_BUF[UTIL_OPBINCFG_OPCODE] = util_opbin2_to_spvop(
					(e_id_default)UTIL_OPBINCFG_RESTYID[LST_SCALE_BUF]
						, (enum UTIL_OPBIN2_)UTIL_OPBINCFG_OPCODE[LST_SCALE_BUF]
						, CTX
						)) {
				err_puts(("SpvOp went undefined"));
				return 1;
			} else {
				dbg_prefix(a);
				dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", LST_SCALE_BUF[5])));
			}

			switch(LST_SCALE_BUF[UTIL_OPBINCFG_MASKLITERAL]) {
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
					XORSWAP(LST_SCALE_BUF[3], LST_SCALE_BUF[5]);

					/** we have four operands (including opcode) */
					LST_SCALE_BUF[3] |= mk_noprnds(4);

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
					XORSWAP(LST_SCALE_BUF[4], LST_SCALE_BUF[5]);
#undef	XORSWAP
					break;

				case UTIL_LITERAL_MASK_LITERAL: 
				case UTIL_LITERAL_LITERAL:
					{
						struct util_literal_eval_ EVAL_LIT 
							= util_literal_eval2(
									LST_EXTRA[0].m_literal
									, LST_EXTRA[1].m_literal
									, (enum SpvOp_)LST_SCALE_BUF[5]
									,(e_id_default)LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]
									);

						if(EVAL_LIT.m_found) {
							dbg_puts(("The result is literal."));
							*RET_LITERAL = rdwr_prv_extra->m_literal = EVAL_LIT.m_ret;
							rdwr_prv_extra->m_mask_literal = 3;
							rdwr_prv_extra->m_type_original = LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID];
							LST_SCALE_BUF[1] = 3;
							switch(aclexpr_arithmetic_cast_literal(
									LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]
									, EVAL_LIT.m_ret
									, rdwr_prv_idval
									, LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]
									, 0, CTX)) {
								case EMIT_EXPR_SUCCESS:
								case EMIT_EXPR_SUCCESS_CONSTANT:
								default:
									ae2f_unreachable();

								case EMIT_EXPR_NOT_THE_CASE:
								case EMIT_EXPR_FAILURE:
									return 1;

								case EMIT_EXPR_SUCCESS_LITERAL:
									break;
							}

							dbg_prefix();
							dbg_call((void), printf("literal id: %u\n", *rdwr_prv_idval));;
							return 0;
						}
					}

					ae2f_fallthrough;
				case UTIL_LITERAL_MASK_CONSTANT: 
					dbg_puts(("The result is constant(fallback)."));
					LST_SCALE_BUF[1] = UTIL_LITERAL_MASK_CONSTANT;
					break;
				default:
					err_puts(("unknown literal"));
					ae2f_unreachable();
			}


#define		TMPL_SECTION	(*((LST_SCALE_BUF[1]) == UTIL_LITERAL_RT ? &CTX->m_section.m_fnimpl	: &CTX->m_section.m_types))
#define		TMPL_COUNT	(*((LST_SCALE_BUF[1]) == UTIL_LITERAL_RT ? &CTX->m_count.m_fnimpl	: &CTX->m_count.m_types))

			dbg_puts("vec_with_copy");
			_aclspv_grow_vec_with_copy(
					_aclspv_malloc
					, _aclspv_free
					, memcpy
					, L_new
					, TMPL_SECTION
					, (size_t)count_to_sz(TMPL_COUNT + (LST_SCALE_BUF[0] + UTIL_OPBINCFG_RESID)
						- (LST_SCALE_BUF[1] == UTIL_LITERAL_RT))
					);

			ae2f_expected_but_else(TMPL_SECTION.m_p) {
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
				return 1;
			}

			dbg_puts("TMPCHECK: Is before memcpy okay?");

#define        LST_CMD_COUNT           (UTIL_OPBINCFG_RESID /** header */ + (LST_SCALE_BUF[0]))

			{
				aclspv_wrd_t	CI = LST_CMD_COUNT - (!LST_SCALE_BUF[1]);
				while(CI --> 0) {
					get_wrd_of_vec(&TMPL_SECTION)[TMPL_COUNT + CI] 
						= LST_SCALE_BUF[(aclspv_wrd_t)UTIL_OPBINCFG_SPECCONSTANTOP + (!LST_SCALE_BUF[1]) + CI];
				}
			}

			TMPL_COUNT += LST_CMD_COUNT - (LST_SCALE_BUF[1] == UTIL_LITERAL_RT);
	}

#undef		LST_CMD_COUNT
#undef		TMPL_SECTION
#undef		TMPL_COUNT

#undef	EXPRSTCK_COUNT
#undef	NUM_CMDCOUNTERSTCK
#undef	CMDSTCK_SCALE
#undef	BUF_EXPRSTCK
#undef	GETLASTELEXPRSTCK
#undef	BUF_CMDCOUNTERSTCK
#undef	GETLASTEL_CMDCOUNTERSTCK

#undef	EXPRSTCK	/** CTX->m_tmp.m_v0 */
#undef	CMDSTCK_SCALE	/** CTX->m_tmp.m_v1 */
#undef	CMDCOUNTERSTCK	/** CTX->m_tmp.m_v2 */
#undef	CTX

	return 0;
}
