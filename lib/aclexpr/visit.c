/** @file visit.c */

#include <util/emitx.h>
#include <util/scale.h>
#include <aclexpr/op.h>

#include <ae2f/cc/memset.h>

#include "./carve.h"

#include "./visit/api.h"
#include "./visit/simple.h"
#include "./visit/op1/arithmetic_cast.h"
#include "./visit/op1/incr.h"
#include "./visit/op2/arithmetic.h"
#include "./visit/op2/assign.h"
#include "./visit/op2/bitwise.h"

enum {
	OP2_ARITHMETIC,
	OP2_BITWISE,
	OP2_ASSIGN,
	OP1_ARITHMETIC_CAST,
	OP1_INCR,
	OP0_,
	OP0_VAL,
	OP_FAIL,
	OP_FALL,
	OP_RECURSE
};

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

#define	CMDSTCK_SCALE	CTX->m_tmp.m_v1
#define	CMDCOUNTERSTCK	CTX->m_tmp.m_v2
#define	EXPRSTCK	CTX->m_tmp.m_v3

	typedef struct {
		aclspv_wrd_t	m_id_req;
	} exprstck;

	typedef struct {
		aclspv_wrd_t	m_count_oprnd;
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
	aclutil_literal* ae2f_restrict RET_LITERAL		= &LITERAL;

	aclexpr_opbincfg_extra* ae2f_restrict	LST_EXTRA;

	dbg_prefix(i);
	dbg_call(ae2f_NONE, (fprintf(stderr, "[visit] OPR: %u/%u ID(scale): %u\n"
					, (unsigned)GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd
					, (unsigned)LST_SCALE_BUF[0]
					, (unsigned)LST_SCALE->m_id
				    )));

	ae2f_ifnezerr(CTX->m_err) {
		err_prefix();
		err_call(ae2f_NONE, (fprintf(stderr, "[visit] error %u\n"
						, (unsigned)CTX->m_err
					    )));
		return CXChildVisit_Break;
	}

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

	LST_EXTRA = (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[0])]);

	ID_REQ = CTX->m_id++;
	LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;

	/** Recurse filter */
	switch((uintmax_t)h_cur.kind) {
		default:
			break;
		case CXCursor_UnexposedExpr:
			return CXChildVisit_Recurse;
	}

	LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_cursor = h_cur;

#if	aclspv_cgoto
#	define	cgswtch(a)		goto LBL_##a;
#	define	cgcsdef(a)		LBL_##a
#	define	cgdefault		cgcsdef
#	define	cgcase(a)		(&&LBL_##a)
#	define	cgjmp(a)		ae2f_cgoto_beg(); goto *(a); ae2f_cgoto_end()
#else
#	define	cgswtch(a)		uint_leastenum_t LABELJMP = a; LABELJMPINIT: switch(LABELJMP)
#	define	cgcsdef(a)		case a
#	define	cgdefault(a)		case a: default
#	define	cgcase(a)		a
#	define	cgjmp(a)		LABELJMP = a; goto LABELJMPINIT
#endif

	dbg_puts("[visit] CGSWTCH");
	cgswtch(OP2_ARITHMETIC) {
		aclspv_wrd_t	N_OPRND_TMP;

		cgcsdef(OP_FAIL):
		{
			err_puts("[visit] OP_FAIL");
			CTX->m_err = (e_aclspv_compile_t) ae2f_bll_sel_bool1(
					uint_leastenum_t
					, CTX->m_err
					, (uint_leastenum_t) CTX-> m_err
					, ACLSPV_COMPILE_UNHANDLED_VISIT);
			return CXChildVisit_Break;
		}

		cgcsdef(OP_RECURSE):
		{
			dbg_prefix();
			dbg_call(, fprintf(stderr, "[visit] OP_RECURSE(%u)\n", N_OPRND_TMP));

			{
				size_t	NEWSZ = EXPRSTCK.m_sz;
				while((size_t)(EXPRSTCK_COUNT + N_OPRND_TMP) > NEWSZ / sizeof(exprstck))
					NEWSZ <<= 1;

				_aclspv_grow_vec_with_copy(
						_aclspv_malloc, _aclspv_free, _aclspv_memcpy
						, dest
						, EXPRSTCK
						, NEWSZ
						);

				ae2f_ifezerr(EXPRSTCK.m_p) {
					CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
					return CXChildVisit_Break;
				}
			}

			(void)(EXPRSTCK_COUNT += N_OPRND_TMP);

			{
				size_t NEWSZ = CMDCOUNTERSTCK.m_sz;
				while(
						((size_t)NUM_CMDCOUNTERSTCK > NEWSZ / sizeof(cmdcounterstckelm)) 
						&& ((NEWSZ <<= 1)) > CMDCOUNTERSTCK.m_sz);

				ae2f_ifezerr(NEWSZ > CMDCOUNTERSTCK.m_sz / sizeof(cmdcounterstckelm)) {
					CTX->m_err = ACLSPV_COMPILE_TOO_BIG;
					return CXChildVisit_Break;
				}

				_aclspv_grow_vec_with_copy(
						_aclspv_malloc,  _aclspv_free, _aclspv_memcpy
						, dest, CMDCOUNTERSTCK
						, NEWSZ
						);

				ae2f_ifezerr(CMDCOUNTERSTCK.m_p)
				{
					CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
					return CXChildVisit_Break;
				}
			}

			(void)++NUM_CMDCOUNTERSTCK;
			GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd = 0;
			return CXChildVisit_Recurse;
		}

		cgcsdef(OP0_):
		{
			dbg_puts("[visit] OP0_");

			if(LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal & (aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL) {
				*RET_LITERAL = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(i).m_count_oprnd].m_literal; 
			}

			IS_NOT_CONSTANT &= LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal;

			LST_SCALE_BUF[ACLEXPR_OPBINCFG_END + GETLASTEL_CMDCOUNTERSTCK(i).m_count_oprnd] = ID_REQ;
			TYPE_CAST_CURRENT = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_type_original;

			(void)(EXPRSTCK_COUNT && --EXPRSTCK_COUNT);
			(void)(LST_SCALE->m_id && ++GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd);

			while(LST_SCALE->m_id && GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd == LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND]) {
				aclexpr_opbincfg_extra	EXTRA_PRV;
				aclspv_wrd_t	IDVAL_PRV = 0;

				ae2fcc_memset(&EXTRA_PRV, 0, sizeof(aclexpr_opbincfg_extra));

				(void)--GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd;

				LST_SCALE		= get_last_scale_from_vec(&CMDSTCK_SCALE);
				LST_SCALE_BUF		= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);

				ae2fpat_assertassume(LST_SCALE_BUF);

				LST_EXTRA
					= (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[0])]);
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

				ae2f_ifezerr(IDVAL_PRV) {
					CTX->m_err = CTX->m_err ? CTX->m_err : ACLSPV_COMPILE_UNHANDLED_VISIT;
					return CXChildVisit_Break;
				}

				del_scale_from_vec_last(&CMDSTCK_SCALE);

				ae2f_ifezerr(NUM_CMDCOUNTERSTCK) {
					err_puts("[visit] command counter stack has been broken.");
					return CXChildVisit_Break;
				} (void)--NUM_CMDCOUNTERSTCK;

				LST_SCALE		= get_last_scale_from_vec(&CMDSTCK_SCALE);
				LST_SCALE_BUF		= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
				LST_EXTRA		= (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[0])]);

				LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd] = EXTRA_PRV;
				ID_REQ = LST_SCALE_BUF[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd + ACLEXPR_OPBINCFG_END] = IDVAL_PRV;
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

			return ae2f_expected_not(CTX->m_err) ? CXChildVisit_Break : CXChildVisit_Continue;
		}

		cgcsdef(OP2_ARITHMETIC): 
		{
			dbg_puts("[visit] OP2_ARITHMETIC");
			N_OPRND_TMP = 2;
			cgjmp(aclexpr_visit_op2_arithmetic(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_BITWISE)
						));
		}

		cgcsdef(OP2_BITWISE):
		{
			dbg_puts("[visit] OP2_BITWISE");
			N_OPRND_TMP = 2;
			cgjmp(aclexpr_visit_op2_bitwise(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_ASSIGN)
						));
		}

		cgcsdef(OP2_ASSIGN):
		{
			dbg_puts("[visit] OP2_ASSIGN");
			N_OPRND_TMP = 2;
			cgjmp(aclexpr_visit_op2_assign(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_ARITHMETIC_CAST)
						));
		}

		cgcsdef(OP1_ARITHMETIC_CAST):
		{
			dbg_puts("[visit] OP1_ARITHMETIC_CAST");
			N_OPRND_TMP = 1;
			cgjmp(aclexpr_visit_op1_arithmetic_cast(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_INCR)
						));
		}

		cgcsdef(OP1_INCR):
		{
			dbg_puts("[visit] OP1_INCR");
			N_OPRND_TMP = 1;
			cgjmp(aclexpr_visit_op1_incr(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP0_VAL)
						));
		}

		cgcsdef(OP0_VAL):
		{
			dbg_puts("[visit] OP0_VAL");
			cgjmp(aclexpr_visit_op0_val(
						h_cur, CTX
						, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd]
						, &ID_REQ
						, cgcase(OP0_)
						, cgcase(OP_FAIL)
						, cgcase(OP_FALL)
						));
		}

		cgdefault(OP_FALL):
		{
			dbg_puts("[visit] OP_FALL");
			CXString SPELL = clang_getCursorSpelling(h_cur);
			CXString KINDSPELL = clang_getCursorKindSpelling(h_cur.kind);

			warn_prefix(i);
			warn_call(fprintf, (stderr, "[visit] OP_FALL::spell(%s)::kind(%s)\n"
						, (const char*)SPELL.data
						, (const char*)KINDSPELL.data));

			clang_disposeString(SPELL);
			clang_disposeString(KINDSPELL);

			return CXChildVisit_Recurse;
		}
	}

	ae2f_unreachable();

#if	WARN



#endif

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
