/** @file alcpsv.c */


#include <aclspv.h>
#include <util/log.h>

#include <.aclspv/util/vcopr.h>

ACLSPV_ABI_IMPL e_aclspv_compile_t  aclspv_compile(
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		aclspv_wrdcount_t* ae2f_restrict			rwr_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict		rwr_output,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
		)
{
	x_aclspv_ctx	CTX; 
#define	STATE_VAL	CTX.m_err

	ae2f_ifezerr(rdwr_unsaved) {
		err_puts("`rdwr_unsaved` is null");
		STATE_VAL = ACLSPV_COMPILE_PRM_INVAL_NIL;
		goto LBL_ERRCLEANUP;
	}

	ae2f_ifezerr(rwr_output) {
		err_puts("`rwr_output` is null");
		STATE_VAL = ACLSPV_COMPILE_PRM_INVAL_NIL;
		goto LBL_ERRCLEANUP;
	}

	ae2f_ifnezerr(aclspv_init_ctx(
				&CTX
				, 30
				, ACLSPV_CTX_FLAG0_SYMBOL | ACLSPV_CTX_FLAG0_OGL | ACLSPV_CTX_FLAG0_LOGICAL | ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS
				, 0x10000
				)) return CTX.m_err;

	ae2f_ifeznrm(aclspv_ctx_compile3(
				&CTX
				, rdwr_unsaved
				, c_unsaved_count
				, rd_argv_opt
				, c_argc
				, rwr_cxerr_opt
				))
		STATE_VAL = aclspv_dump_ctx_output(
				&CTX
				, rwr_output_count_opt
				, rwr_output);

	goto LBL_CLEANUP;

LBL_ERRCLEANUP:
	CTX.m_err = ae2f_bll_sel_bool_naive1(
			uint_fastenum_t
			, CTX.m_err
			, CTX.m_err
			, ACLSPV_COMPILE_UNHANDLED
			);

LBL_CLEANUP:
	aclspv_fini_ctx(&CTX);
	return CTX.m_err;
}


