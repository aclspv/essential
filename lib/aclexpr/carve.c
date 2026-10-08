#include "./carve.h"
#include "./carve/util.h"
#include "./carve/op1/arithmetic_cast.h"
#include "./carve/op1/deref.h"
#include "./carve/op1/incr.h"
#include "./carve/op1/memberread.h"
#include "./carve/op1/subscript.h"
#include "./carve/opn/initlist.h"
#include "./carve/op2/arithmetic.h"
#include "./carve/op2/cmp.h"
#include "./carve/op2/shift.h"
#include "./carve/op2/assign.h"
#include "./carve/op2/derefwrite.h"
#include "./carve/op2/memberwrite.h"
#include "./carve/op2/bool0.h"
#include "./carve/op2/bool1.h"

#include <aclexpr.h>

#include <util/wrdemit.h>
#include <util/scale.h>

#include <ae2f/cgoto.h>
#include <ae2f/pat/dbg.h>

enum {
	OP_FAILURE,
	OP_SUCCESS,

	OP2_,
	OP2_ARITHMETIC,
	OP2_CMP,
	OP2_SHIFT,
	OP2_BOOL0,
	OP2_BOOL1,
	OP2_ASSIGN,
	OP2_MEMBERWRITE,
	OP2_DEREFWRITE,
	OP2_SUBSCRIPT,
	OP2_SUBSCRIPTWRITE,
	OP1_,
	OP1_ARITHMETIC_CAST,
	OP1_INCR,
	OP1_DEREF,
	OP1_ADDRESS,
	OP1_MEMBERREAD,
	OPN_INITLIST
};

aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		) {
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE(CTX(rdwr_data)));
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(
			aclspv_wrd_t
			, &CMDSTCK_SCALE(CTX(rdwr_data))
			, *LST_SCALE
			);
	aclutil_literal* ae2f_restrict RET_LITERAL
		= &RET_LITERAL_REF(rdwr_data);

	aclspv_wrd_t	NUM_OPRND;

	aclexpr_opbincfg_extra* ae2f_restrict LST_EXTRA;

	ae2fpat_assertassume(LST_SCALE_BUF);
	ae2fpat_assertassume(rdwr_prv_extra);
	ae2fpat_assertassume(rdwr_prv_idval);

	NUM_OPRND = LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND];
	LST_EXTRA = (void* ae2f_restrict)(LST_SCALE_BUF + NUM_OPRND + ACLEXPR_OPBINCFG_END);

	/****************************************************
	 * Contributor: https://github.com/dxdxffgg99
	 *
	 * Message:
	 * I'm sorry. 
	 * I shouldn't have shown Dalmuri the VM code I made. 
	 * I'll gladly accept any criticism, so please contact me on Discord. 
	 * 	-The person who showed Dalmurii the Computed Goto-
	 *
	 **************************************************** */
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

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "[aclexpr::carve] oprnd %u opcode %u\n"
				, (unsigned)LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND]
				, (unsigned)LST_SCALE_BUF[ACLEXPR_OPBINCFG_OPCODE]));

	cgswtch(OPN_INITLIST) {
		cgdefault(OP_FAILURE):
			err_prefix();
		err_call((void), fprintf(stderr, "something from somewhere %u\n", (unsigned)LST_SCALE_BUF[0]));
		rdwr_data->m_ctx->m_err = rdwr_data->m_ctx->m_err ? rdwr_data->m_ctx->m_err : ACLSPV_COMPILE_UNHANDLED_CARVE;
		return 1;

		cgcsdef(OP_SUCCESS):
			return 0;

		cgcsdef(OPN_INITLIST):
			dbg_puts("[carve] OPN_INITLIST");
		cgjmp(aclexpr_carve_opn_initlist(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_):
			cgjmp((LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND] == 2 ? cgcase(OP2_ARITHMETIC) : cgcase(OP1_)));

		cgcsdef(OP2_ARITHMETIC):
			dbg_puts("[carve] OP2_ARITHMETIC");
		cgjmp(aclexpr_carve2_arithmetic(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_CMP)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_CMP):
			dbg_puts("[carve] OP2_CMP");
		cgjmp(aclexpr_carve2_cmp(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_SHIFT)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_SHIFT):
			dbg_puts("[carve] OP2_SHIFT");
		cgjmp(aclexpr_carve2_shift(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_BOOL0)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_BOOL0):
			dbg_puts("[carve] OP2_BOOL0");
		cgjmp(aclexpr_carve2_bool0(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_BOOL1)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_BOOL1):
			dbg_puts("[carve] OP2_BOOL0");
		cgjmp(aclexpr_carve2_bool1(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_ASSIGN)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_ASSIGN):
			dbg_puts("[carve] OP2_ASSIGN");
		cgjmp(aclexpr_carve2_assign(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_DEREFWRITE)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_DEREFWRITE):
			dbg_puts("[carve] OP2_DEREFWRITE");
		cgjmp(aclexpr_carve2_derefwrite(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_SUBSCRIPT)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_SUBSCRIPT):
			dbg_puts("[carve] OP2_SUBSCRIPT");
		cgjmp(aclexpr_carve2_subscript(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_SUBSCRIPTWRITE)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_SUBSCRIPTWRITE):
			dbg_puts("[carve] OP2_SUBSCRIPTWRITE");
		cgjmp(aclexpr_carve2_subscriptwrite(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_MEMBERWRITE)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP2_MEMBERWRITE):
			dbg_puts("[carve] OP2_MEMBERWRITE");
		cgjmp(aclexpr_carve2_memberwrite(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP_FAILURE)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP1_):
			cgjmp(LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND] == 1 ? cgcase(OP1_ARITHMETIC_CAST) : cgcase(OP_FAILURE));

		cgcsdef(OP1_ARITHMETIC_CAST):
			dbg_puts("[carve] OP1_ARITHMETIC_CAST");
		cgjmp(aclexpr_carve1_arithmetic_cast(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP1_INCR)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP1_INCR):
			dbg_puts("[carve] OP1_INCR");
		cgjmp(aclexpr_carve1_incr(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP1_DEREF)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP1_DEREF):
			dbg_puts("[carve] OP1_DEREF");
		cgjmp(aclexpr_carve1_deref(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP1_ADDRESS)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP1_ADDRESS):
			dbg_puts("[carve] OP1_ADDRESS");
		cgjmp(aclexpr_carve1_address(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP1_MEMBERREAD)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));

		cgcsdef(OP1_MEMBERREAD):
			dbg_puts("[carve] OP1_MEMBERREAD");
		cgjmp(aclexpr_carve1_memberread(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP_FAILURE)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));
	}

	ae2f_unreachable();
}

