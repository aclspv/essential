/** @file visit.c */

#include <clang-c/Index.h>
#include <util/emitx.h>
#include <util/scale.h>
#include <util/op.h>

#include <ae2f/cc/memset.h>

#include "./visit.h"
#include "./carve.h"
#include "aclspv.h"
#include "ae2f/cc.h"
#include "util/log.h"

#include "./visit/simple.h"
#include "./visit/cast_numeric.h"
#include "./visit/basic_numeric_operation.h"

enum CXChildVisitResult aclexpr_visit(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData rdwr_data
		)
{
#define	BASE			((struct aclexpr_visit_data* ae2f_restrict)(rdwr_data))
#define	CTX			BASE->m_ctx
#define EXPRSTCK_COUNT		BASE->m_exprcount 
#define ID_REQ			BASE->m_id_ret 
#define	IS_NOT_CONSTANT		BASE->m_literal_flag 
#define	TYPE_CAST_CURRENT	BASE->m_tyid_current 
#define	NUM_CMDCOUNTERSTCK	BASE->m_num_cmdcounter
#define	LITERAL			BASE->m_ret_literal

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
	util_literal* ae2f_restrict RET_LITERAL		= &LITERAL;

	util_opbincfg_extra* ae2f_restrict	LST_EXTRA;

	ae2f_ifnezerr(CTX->m_err) return CXChildVisit_Break;

	if(EXPRSTCK_COUNT) {
		ae2f_expected_but_else(EXPRSTCK.m_p) {
			CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
			return CXChildVisit_Break;
		}
	}

	ae2f_ifezerr(NUM_CMDCOUNTERSTCK) {
		CTX->m_err = ACLSPV_COMPILE_TOO_BIG_EXPRVISIT;
		return CXChildVisit_Break;
	}

	ae2f_expected_but_else(LST_SCALE_BUF) {
		CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
		return CXChildVisit_Break;
	}

	LST_EXTRA = (void* ae2f_restrict)(&LST_SCALE_BUF[(UTIL_OPBINCFG_END + LST_SCALE_BUF[0])]);

	dbg_prefix(i);
	dbg_call(ae2f_NONE, (fprintf(stderr, "Operand Counter: %u/%u[%lu]\n"
					, (unsigned)GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd
					, (unsigned)LST_SCALE_BUF[0]
					, (unsigned long)LST_SCALE->m_id
				    )));
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
			CTX->m_err = CTX->m_err ? CTX->m_err : ACLSPV_COMPILE_UNHANDLED_BODY;
			return CXChildVisit_Break;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_CONSTANT:
		case EMIT_EXPR_SUCCESS_LITERAL:
			while((size_t)(EXPRSTCK_COUNT + 2) > EXPRSTCK.m_sz / sizeof(exprstck)) {
				_aclspv_grow_vec_with_copy(
						_aclspv_malloc,  _aclspv_free, _aclspv_memcpy
						, dest, EXPRSTCK, EXPRSTCK.m_sz << 1
						);

				ae2f_expected_but_else(EXPRSTCK.m_p) {
					CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
					return CXChildVisit_Break;
				}
			}

			EXPRSTCK_COUNT += 2;

			while((size_t)NUM_CMDCOUNTERSTCK >= CMDCOUNTERSTCK.m_sz / sizeof(cmdcounterstckelm)) {
				_aclspv_grow_vec_with_copy(
						_aclspv_malloc,  _aclspv_free, _aclspv_memcpy
						, dest, CMDCOUNTERSTCK
						, CMDCOUNTERSTCK.m_sz << 1
						);

				ae2f_expected_but_else(CMDCOUNTERSTCK.m_p) {
					CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
					return CXChildVisit_Break;
				}
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
			CTX->m_err = ae2f_bll_sel_bool1(
					uint_leastenum_t
					, CTX->m_err
					, CTX->m_err
					, ACLSPV_COMPILE_UNHANDLED_VISIT_SIMPLE);
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
				aclspv_bool_t	IS_ASSIGN	= LST_EXTRA[0].m_is_assign;

				ae2fcc_memset(&EXTRA_PRV, 0, sizeof(util_opbincfg_extra));

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
								, (unsigned long)*((size_t*)CMDSTCK_SCALE.m_p)
								, (void*)&LST_SCALE->m_id
								, (unsigned long)LST_SCALE->m_id
							 )));
				dbg_prefix(a);
				dbg_call((void), (fprintf(
								stderr
								, "count operand: %u\n"
								, LST_SCALE_BUF[0] - 1
							 )));

				ae2f_unexpected_but_if(aclexpr_carve(
							rdwr_data
							, &IDVAL_PRV
							, &EXTRA_PRV))
					return CXChildVisit_Break;

				if(IS_ASSIGN) {
					CXCursor	CASSIGN		= LST_EXTRA[0].m_cursor;
					aclspv_wrd_t IDX = aclutil_find_cursor(
							CTX->m_num_cursor
							, CTX->m_cursors.m_p
							, clang_getCanonicalCursor(clang_getCursorDefinition(CASSIGN)));


					aclspv_wrd_t	CLVAL;

					dbg_prefix(a);
					dbg_call((void), fprintf(stderr, "assign after carve %u in type %u\n"
								, IDVAL_PRV
								, EXTRA_PRV.m_type_original
								));

					CLVAL = aclutil_assign_cursor_lval(
							&((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
							, CTX, IDVAL_PRV, EXTRA_PRV.m_type_original
							, EXTRA_PRV.m_mask_literal
							, &EXTRA_PRV.m_literal, 0);

					ae2f_ifezerr(CLVAL) {
						err_puts("[carve] aclutil_assign_cursor_lval has been failed.");
					}

					dbg_prefix();
					dbg_call((void), fprintf(stderr
								, "[carve::%u] after assign %u with type %u\n"
								, CLVAL
								, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_data.m_vardecl.m_init_val
								, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_data.m_vardecl.m_type_id
								));
				}

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

			return ae2f_bll_sel_bool_naive1(uint_leastenum_t, CTX->m_err, CXChildVisit_Break, CXChildVisit_Continue);

		case EMIT_EXPR_NOT_THE_CASE:
			dbg_puts("[nocase] s_simple");
			break;
		default:
			ae2f_unreachable();
			CTX->m_err = ACLSPV_COMPILE_MET_INVAL_UNREACHABLE;
			return CXChildVisit_Break;
	}

	switch(s_cast_numeric(
				h_cur
				, CTX
				, &CMDSTCK_SCALE
			     )) {
		default:
		case EMIT_EXPR_FAILURE:
			CTX->m_err = CTX->m_err ? CTX->m_err : ACLSPV_COMPILE_UNHANDLED_BODY;
			return CXChildVisit_Break;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_CONSTANT:
		case EMIT_EXPR_SUCCESS_LITERAL:
			_aclspv_grow_vec_with_copy(
					_aclspv_malloc
					, _aclspv_free
					, _aclspv_memcpy
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
