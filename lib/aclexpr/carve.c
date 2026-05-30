#include "./carve.h"
#include <util/wrdemit.h>
#include <util/scale.h>
#include <util/cursor.h>
#include <aclexpr.h>
#include <ae2f/cgoto.h>
#include "./carve/util.h"
#include "./carve/op1/arithmetic_cast.h"
#include "./carve/op2/arithmetic.h"

enum {
	OP_FAILURE,
	OP_SUCCESS,

	OP2_,
	OP2_ARITHMETIC,

	OP1_,
	OP1_ARITHMETIC_CAST
};

aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
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

	util_opbincfg_extra* ae2f_restrict LST_EXTRA;

	ae2fpat_assertassume(LST_SCALE_BUF);
	ae2fpat_assertassume(rdwr_prv_extra);
	ae2fpat_assertassume(rdwr_prv_idval);

	NUM_OPRND = LST_SCALE_BUF[0];
	LST_EXTRA = (void* ae2f_restrict)(LST_SCALE_BUF + NUM_OPRND + UTIL_OPBINCFG_END);

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

	cgswtch(OP2_) {
		cgdefault(OP_FAILURE):
			return 1;

		cgcsdef(OP_SUCCESS):
			return 0;

		cgcsdef(OP2_):
			cgjmp(LST_SCALE_BUF[ACLUTIL_OPBINCFG_NUMOPRND] == 2 ? cgcase(OP2_ARITHMETIC) : cgcase(OP1_));

		cgcsdef(OP2_ARITHMETIC):
			ae2fpat_assertassume(LST_SCALE_BUF[ACLUTIL_OPBINCFG_NUMOPRND] == 2);
		cgjmp(aclexpr_carve_op2_arithmetic(
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
			cgjmp((LST_SCALE_BUF[ACLUTIL_OPBINCFG_NUMOPRND] == 1) ? cgcase(OP1_ARITHMETIC_CAST) : cgcase(OP_FAILURE));

		cgcsdef(OP1_ARITHMETIC_CAST):
			ae2fpat_assertassume(LST_SCALE_BUF[ACLUTIL_OPBINCFG_NUMOPRND] == 1);
		cgjmp(aclexpr_carve_op1_arithmetic_cast(
					rdwr_data
					, rdwr_prv_idval
					, rdwr_prv_extra
					, LST_SCALE
					, LST_SCALE_BUF
					, LST_EXTRA
					, RET_LITERAL
					, cgcase(OP2_ARITHMETIC)
					, cgcase(OP_SUCCESS)
					, cgcase(OP_FAILURE)
					));
	}

	ae2f_unreachable();
}
