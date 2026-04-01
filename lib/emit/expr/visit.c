/** @file visit.c */

#include <clang-c/Index.h>
#include <util/emitx.h>
#include <util/scale.h>
#include <util/op.h>

#include "./visit.h"
#include "aclspv/abi.h"
#include "aclspv/spvty.h"
#include "ae2f/pat/dbg.h"
#include "emit/expr.h"
#include "util/cursor.h"
#include "util/literal.h"
#include "util/log.h"

#if	1
#include "./visit/simple.h"
#include "./visit/cast_numeric.h"
#include "./visit/basic_numeric_operation.h"
#endif

ae2f_inline static int s_carve_one_scale(
		uintptr_t* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		) {
#define CTX			((h_util_ctx_t)((rdwr_data))[0])
#define EXPRSTCK_COUNT		(((rdwr_data))[1])
#define	NUM_CMDCOUNTERSTCK	(((rdwr_data))[5])

#define	EXPRSTCK	CTX->m_tmp.m_v0
#define	CMDSTCK_SCALE	CTX->m_tmp.m_v1
#define	CMDCOUNTERSTCK	CTX->m_tmp.m_v2
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE);
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
	util_literal* ae2f_restrict RET_LITERAL
		= (void*)(((uptr* ae2f_restrict)(rdwr_data)) + 6);

	aclspv_wrd_t	NUM_OPRND;
	util_opbincfg_extra* ae2f_restrict LST_EXTRA; 


	typedef struct {
		u32_least	m_id_req;
	} exprstck;

	typedef struct {
		u32_least	m_count_oprnd;
	} cmdcounterstckelm;

	aclspv_bool_t		OPCODE_ASSIGN;

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
			OPCODE_ASSIGN = util_opbin2_is_assign(LST_SCALE_BUF[UTIL_OPBINCFG_OPCODE]);
			dbg_prefix(a);
			dbg_call((void), printf("OPCODE[%u]_ASSIGN: %u\n", LST_SCALE_BUF[UTIL_OPBINCFG_OPCODE], OPCODE_ASSIGN));

			unless(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID])
				LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID] = aclid_decide_cast(
						LST_EXTRA[0].m_type_original
						, LST_EXTRA[1].m_type_original
						);	/** only when bin2 (arithmetic thing) */

			rdwr_prv_extra->m_type_original = LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID];
			if(aclid_is_float(rdwr_prv_extra->m_type_original)) {
				rdwr_prv_extra->m_mask_literal &= ~(aclspv_wrd_t)UTIL_LITERAL_CONSTANT;
			}

			ae2fpat_assertassume_strict(LST_SCALE_BUF);
			ae2fpat_assertassume_strict(LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]);

			dbg_prefix(a);
			dbg_call((void), fprintf(stderr, "NUM_OPRND: %u\n", NUM_OPRND));


			/**** KERNEL SHIT *****/
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
						UTIL_OPBINCFG_RESTYID[LST_SCALE_BUF]
						, UTIL_OPBINCFG_OPCODE[LST_SCALE_BUF]
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

							if(OPCODE_ASSIGN) {
								CXCursor	RFD = clang_getCursorReferenced(LST_EXTRA[0].m_cursor);
								const aclspv_wrd_t IDX = aclutil_find_cursor(
										CTX->m_num_cursor
										, CTX->m_cursors.m_p
										, RFD
										);

								/** FIXME: error */
								ae2f_unexpected_but_if(IDX == CTX->m_num_cursor)
									return 1;

								ae2f_expected_but_else(aclutil_assign_cursor_lval(
										&((util_cursor*)CTX->m_cursors.m_p)[IDX]
										, CTX
										, 0
										, LST_SCALE_BUF[UTIL_OPBINCFG_RESTYID]
										, 2
										, &rdwr_prv_extra->m_literal
										, 0)) return 1;
							}

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
					ae2fpat_assertassume(0);
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
			memcpy(
					get_wrd_of_vec(&TMPL_SECTION) + TMPL_COUNT
					, LST_SCALE_BUF + UTIL_OPBINCFG_SPECCONSTANTOP + (!LST_SCALE_BUF[1])
					, (size_t)count_to_sz(LST_CMD_COUNT - (!LST_SCALE_BUF[1]))
			      );

			TMPL_COUNT += LST_CMD_COUNT - (LST_SCALE_BUF[1] == UTIL_LITERAL_RT);

			if(OPCODE_ASSIGN) {
				CXCursor	RFD = clang_getCursorReferenced(LST_EXTRA[0].m_cursor);
				const aclspv_wrd_t IDX = aclutil_find_cursor(
						CTX->m_num_cursor
						, CTX->m_cursors.m_p
						, RFD
						);

				ae2f_unexpected_but_if(IDX == CTX->m_num_cursor)
					return 1;

				ae2f_expected_but_else(aclutil_assign_cursor_lval(
							((util_cursor* ae2f_restrict)CTX->m_cursors.m_p) + IDX
							, CTX
							, *rdwr_prv_idval
							, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_data.m_var_simple.m_type_id
							, LST_EXTRA[0].m_mask_literal
							, 0, 0)) return 1;

				dbg_puts("OPCODE_ASSIGN");
			}
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

enum CXChildVisitResult aclexpr_visit(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData rdwr_data
		)
{
#define	CTX			((h_util_ctx_t)(((uptr* ae2f_restrict)rdwr_data)[0]))
#define EXPRSTCK_COUNT		(((uptr* ae2f_restrict)(rdwr_data))[1])
#define ID_REQ			(*((aclspv_wrd_t* ae2f_restrict)(void* ae2f_restrict)&(((uptr* ae2f_restrict)(rdwr_data))[2])))
#define	IS_NOT_CONSTANT		((uptr* ae2f_restrict)(rdwr_data))[3]
#define	TYPE_CAST_CURRENT	(*(aclspv_wrd_t*)&(((uptr* ae2f_restrict)(rdwr_data))[4]))
#define	NUM_CMDCOUNTERSTCK	(((uptr* ae2f_restrict)(rdwr_data))[5])

	/** for stack tmp.m_v0 will be used. */
#define	EXPRSTCK	CTX->m_tmp.m_v0
#define	CMDSTCK_SCALE	CTX->m_tmp.m_v1
#define	CMDCOUNTERSTCK	CTX->m_tmp.m_v2

	typedef struct {
		u32_least	m_id_req;
	} exprstck;

	typedef struct {
		u32_least	m_count_oprnd;
	} cmdcounterstckelm;

	ae2f_unused typedef char s_ASSRT_ELMSIZE[
		sizeof(exprstck) == sizeof(cmdcounterstckelm) && sizeof(cmdcounterstckelm) == sizeof(u32_least)
	];

#define	BUF_EXPRSTCK(a)	((exprstck* ae2f_restrict)(EXPRSTCK).m_p)
#define	GETLASTELEXPRSTCK(a)	BUF_EXPRSTCK(c_stck)[EXPRSTCK_COUNT - 1]
#define	BUF_CMDCOUNTERSTCK(a)	((cmdcounterstckelm* ae2f_restrict)(CMDCOUNTERSTCK).m_p)
#define	GETLASTEL_CMDCOUNTERSTCK(a)	BUF_CMDCOUNTERSTCK()[NUM_CMDCOUNTERSTCK - 1]

	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE);
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
	util_literal* ae2f_restrict RET_LITERAL
		= (void*)(((uptr* ae2f_restrict)(rdwr_data)) + 6);

	util_opbincfg_extra* ae2f_restrict	LST_EXTRA;

	if(EXPRSTCK_COUNT) {
		ae2f_expected_but_else(EXPRSTCK.m_p) {
			CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
			assert(0 && "stack is acting funny");
			return CXChildVisit_Break;
		}
	}

	ae2f_expected_but_else(LST_SCALE_BUF) {
		CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
		return CXChildVisit_Break;
	}

	LST_EXTRA = (void* ae2f_restrict)(&LST_SCALE_BUF[(UTIL_OPBINCFG_END + LST_SCALE_BUF[0])]);

#if 1
	dbg_prefix(i);
	dbg_call(ae2f_NONE, (fprintf(stderr, "Operand Counter: %u/%u[%lu]\n"
					, GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd
					, LST_SCALE_BUF[0]
					, LST_SCALE->m_id
				    )));
#endif
	ID_REQ = CTX->m_id++;
	LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal = UTIL_LITERAL_MASK_LITERAL;

	/** Recurse filter */
	switch((uintmax_t)h_cur.kind) {
		default:
			break;
		case CXCursor_UnexposedExpr:
			return CXChildVisit_Recurse;
	}

	LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_cursor = h_cur;

	switch(s_basic_numeric_operation(
				h_cur
				, CTX
				, &CMDSTCK_SCALE
				))
	{
		default:
		case EMIT_EXPR_FAILURE:
			assert(0 && "FAILURE_expr_bin2");
			return CXChildVisit_Break;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_CONSTANT:
		case EMIT_EXPR_SUCCESS_LITERAL:
			_aclspv_grow_vec_with_copy(
					_aclspv_malloc,  _aclspv_free, memcpy
					, dest, EXPRSTCK, (size_t)(sizeof(exprstck) * (EXPRSTCK_COUNT + 2))
					);

			ae2f_expected_but_else(EXPRSTCK.m_p) {
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
				return CXChildVisit_Break;
			}

			EXPRSTCK_COUNT += 2;

			_aclspv_grow_vec_with_copy(
					_aclspv_malloc,  _aclspv_free, memcpy
					, dest, CMDCOUNTERSTCK
					, (size_t)(sizeof(cmdcounterstckelm) * (NUM_CMDCOUNTERSTCK + 1))
					);

			ae2f_expected_but_else(CMDCOUNTERSTCK.m_p) {
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
				return CXChildVisit_Break;
			}

			++NUM_CMDCOUNTERSTCK;
			GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd = 0;
			return CXChildVisit_Recurse;

		case EMIT_EXPR_NOT_THE_CASE:
			break;
	}

	switch(s_simple(
				h_cur
				, CTX
				, &ID_REQ
				, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_type_original
				, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_literal
		       )) {
		case EMIT_EXPR_FAILURE:
			assert(0 && "bin_1 is fishy");
			CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
			return CXChildVisit_Break;


		case EMIT_EXPR_SUCCESS:
			dbg_prefix(a);
			dbg_call(fprintf, (stderr, "s_bin_1: newid: %u\n", ID_REQ));

			IS_NOT_CONSTANT = UTIL_LITERAL_RT;
			LST_SCALE_BUF[1] = UTIL_LITERAL_MASK_RT;
			LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal = 0;

			ae2f_unexpected_but_if(0) {
				ae2f_unreachable();
				case EMIT_EXPR_SUCCESS_LITERAL:
				*RET_LITERAL = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(i).m_count_oprnd].m_literal; 
			}

			ae2f_unexpected_but_if(0) {
				ae2f_unreachable();
				case EMIT_EXPR_SUCCESS_CONSTANT:

				IS_NOT_CONSTANT &= UTIL_LITERAL_MASK_CONSTANT;
				LST_SCALE_BUF[1] &= UTIL_LITERAL_MASK_CONSTANT;
				LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal &= UTIL_LITERAL_MASK_CONSTANT;
			}

			LST_SCALE_BUF[UTIL_OPBINCFG_END + GETLASTEL_CMDCOUNTERSTCK(i).m_count_oprnd] = ID_REQ;
			TYPE_CAST_CURRENT = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_type_original;

			(void)(EXPRSTCK_COUNT && --EXPRSTCK_COUNT);
			(void)(LST_SCALE->m_id && ++GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd);

			while(LST_SCALE->m_id && GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd == LST_SCALE_BUF[0]) {
				util_opbincfg_extra	EXTRA_PRV;
				aclspv_wrd_t	IDVAL_PRV;

				(void)--GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd;

				LST_SCALE		= get_last_scale_from_vec(&CMDSTCK_SCALE);
				LST_SCALE_BUF		= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);

				ae2fpat_assertassume(LST_SCALE_BUF);

				LST_EXTRA
					= (void* ae2f_restrict)(&LST_SCALE_BUF[(UTIL_OPBINCFG_END + LST_SCALE_BUF[0])]);
				dbg_prefix(a);
				dbg_call((void), (fprintf(
								stderr
								, "Last scale ID[%lu]: [%p]%lu\n"
								, *((size_t*)CMDSTCK_SCALE.m_p)
								, (void*)&LST_SCALE->m_id
								, LST_SCALE->m_id
							 )));
				dbg_prefix(a);
				dbg_call((void), (fprintf(
								stderr
								, "count operand: %u\n"
								, LST_SCALE_BUF[0] - 1
							 )));
				ae2f_unexpected_but_if(s_carve_one_scale(
							rdwr_data
							, &IDVAL_PRV
							, &EXTRA_PRV))
					return CXChildVisit_Break;

				del_scale_from_vec_last(&CMDSTCK_SCALE);
				(void)(NUM_CMDCOUNTERSTCK && --NUM_CMDCOUNTERSTCK);

				LST_SCALE		= get_last_scale_from_vec(&CMDSTCK_SCALE);
				LST_SCALE_BUF		= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);

				LST_EXTRA
					= (void* ae2f_restrict)(&LST_SCALE_BUF[(UTIL_OPBINCFG_END + LST_SCALE_BUF[0])]);

				LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd] = EXTRA_PRV;
				ID_REQ = LST_SCALE_BUF[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd + UTIL_OPBINCFG_END] = IDVAL_PRV;
				TYPE_CAST_CURRENT = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_type_original;
				IS_NOT_CONSTANT	&= LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal;
				*RET_LITERAL = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_literal;

				dbg_prefix(a);
				dbg_call((void), (fprintf(stderr, "LST_EXTRA(%u): %p %u\n"
								, GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd
								, (void*)&LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd].m_mask_literal
								, LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal
							 )));

				(void)(LST_SCALE->m_id && ++GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd);
			}


			return CXChildVisit_Continue;

		case EMIT_EXPR_NOT_THE_CASE:
			dbg_puts("Operator 1: not the case");
			break;
		default:
			ae2fpat_assertassume(0);
	}

	switch(s_cast_numeric(
				h_cur
				, CTX
				, &CMDSTCK_SCALE
			     )) {
		default:
		case EMIT_EXPR_FAILURE:
			assert(0 && "FAILURE_expr_bin1");
			return CXChildVisit_Break;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_CONSTANT:
		case EMIT_EXPR_SUCCESS_LITERAL:
			_aclspv_grow_vec_with_copy(
					_aclspv_malloc
					, _aclspv_free
					, memcpy
					, dest
					, EXPRSTCK
					, (size_t)(sizeof(exprstck) * (EXPRSTCK_COUNT + 1))
					);

			ae2f_expected_but_else(EXPRSTCK.m_p) {
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
				return CXChildVisit_Break;
			}

			(void)++EXPRSTCK_COUNT;

			_aclspv_grow_vec_with_copy(
					_aclspv_malloc, _aclspv_free, memcpy
					, dest, CMDCOUNTERSTCK
					, (size_t)(sizeof(cmdcounterstckelm) * (NUM_CMDCOUNTERSTCK + 1))
					);

			ae2f_expected_but_else(CMDCOUNTERSTCK.m_p) {
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
				return CXChildVisit_Break;
			}

			(void)++NUM_CMDCOUNTERSTCK;
			GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd = 0;

			return CXChildVisit_Recurse;

		case EMIT_EXPR_NOT_THE_CASE:
			break;
	}

	CTX->m_err = ACLSPV_COMPILE_OK;

	dbg_prefix(a);
	dbg_call(ae2f_NONE, (fprintf(stderr, "Type Casted Current: %u\n", TYPE_CAST_CURRENT)));


#if	WARN

	{
		CXString SPELL = clang_getCursorSpelling(h_cur);
		CXString KINDSPELL = clang_getCursorKindSpelling(h_cur.kind);

		warn_prefix(i);
		warn_call(fprintf, (stderr, "Met unprocessed spell(%s) in kind(%s) [NOIMPL, EXPR]\n"
					, (const char*)SPELL.data
					, (const char*)KINDSPELL.data));

		clang_disposeString(SPELL);
		clang_disposeString(KINDSPELL);
	}

#endif

	return CXChildVisit_Recurse;

#undef	CTX
#undef	EXPRSTCK_COUNT
#undef	EXPRSTCK
#undef	CMDSTCK_SCALE
#undef	ID_REQ
#undef	LST_CMD_COUNT
#undef	LST_CMDBUF_COUNT
#undef	BUF_EXPRSTCK
#undef	GETLASTELEXPRSTCK
#undef	CMDCOUNTERSTCK
#undef	NUM_CMDCOUNTERSTCK
#undef	GETLASTEL_CMDCOUNTERSTCK
#undef	BUF_CMDCOUNTERSTCK
}
