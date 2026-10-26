#include <aclspv.h>

#include <util/log.h>

#include <aclemit/root.h>
#include <aclattr/main.h>
#include <aclctx/err.h>

#include <aclutil/capability.h>

ACLSPV_ABI_IMPL aclspv_compile_t aclspv_ctx_compile(h_aclspv_ctx_t h_ctx, CXCursor c_cur_root) {
	h_ctx->m_work_data = h_ctx;

	ae2f_ifezerr(h_ctx)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err) return h_ctx->m_err;

	switch(h_ctx->m_usrcfg.m_mdl_addr) {
		case SpvAddressingModelLogical:
		case SpvAddressingModelMax:
		default:
			break;

		case SpvAddressingModelPhysicalStorageBuffer64:
			ae2f_ifneznrm_strict(aclutil_add_capability_by_flag(
						h_ctx
						, SpvCapabilityPhysicalStorageBufferAddresses)) ;
			else ae2f_ifnezerr_strict(1) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return h_ctx->m_err;
			}

			ae2f_ifnezerr_strict(h_ctx->m_usrcfg.m_ver_spv < aclspv_ver_spv(1, 5)) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return h_ctx->m_err;
			}

			break;

		case SpvAddressingModelPhysical32:
		case SpvAddressingModelPhysical64:
			ae2f_ifneznrm_strict(aclutil_add_capability_by_flag(
						h_ctx
						, SpvCapabilityAddresses)) ;
			else ae2f_ifnezerr_strict(1) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return h_ctx->m_err;
			}

			break;
	}

	switch(h_ctx->m_usrcfg.m_mdl_mem) {
		case SpvMemoryModelMax:
		default:
			break;

		case SpvMemoryModelSimple:
		case SpvMemoryModelGLSL450:
			ae2f_ifneznrm_strict(aclutil_add_capability_by_flag(
						h_ctx
						, SpvCapabilityShader)) ;
			else ae2f_ifnezerr_strict(1) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return h_ctx->m_err;
			}

			break;

		case SpvMemoryModelOpenCL:
			ae2f_ifneznrm_strict(aclutil_add_capability_by_flag(
						h_ctx
						, SpvCapabilityKernel)) ;
			else ae2f_ifnezerr_strict(1) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return h_ctx->m_err;
			}

			break;

		case SpvMemoryModelVulkan:
			ae2f_ifneznrm_strict(aclutil_add_capability_by_flag(
						h_ctx
						, SpvCapabilityVulkanMemoryModel)) ;
			else ae2f_ifnezerr_strict(1) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return h_ctx->m_err;
			}

			break;
	}

	h_ctx->m_work_nxt	= aclemit_root;
	h_ctx->m_work_data	= h_ctx;

	clang_visitChildren(c_cur_root, s_aclattr_main, h_ctx);

	ae2f_ifnezerr_strict(h_ctx->m_work_nxt != aclemit_root)  {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_NOT_INIT_STATE);
	}

	return h_ctx->m_err;
}

ae2f_extern ACLSPV_ABI_DECL aclspv_compile_t aclspv_ctx_compile1(
		const h_aclspv_ctx_t	h_ctx,
		CXTranslationUnit	h_tu
		) {
	unsigned	CXTU_IDX_ERR;

	ae2f_ifezerr(h_ctx)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(h_tu)	return ACLSPV_COMPILE_ERR_CXTU;

	CXTU_IDX_ERR = clang_getNumDiagnostics(h_tu);
	if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS) while(CXTU_IDX_ERR-- && !h_ctx->m_err) {
		const CXDiagnostic DIAG = clang_getDiagnostic(h_tu, CXTU_IDX_ERR);
		enum CXDiagnosticSeverity SEVERITY = clang_getDiagnosticSeverity(DIAG);

		CXString TXT = clang_formatDiagnostic(DIAG, clang_defaultDiagnosticDisplayOptions());

		dbg_puts(TXT.data);

		switch(SEVERITY) {
			case CXDiagnostic_Note:
				dbg_puts("NOTE");
				break;
			case CXDiagnostic_Error:
				if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_KILL_ON_DIAGNOSTICS)
					h_ctx->m_err = ACLSPV_COMPILE_DIAG_ERR;
				dbg_puts("_ERR");
				break;

			case CXDiagnostic_Warning:
				dbg_puts("WARN");
				break;
			case CXDiagnostic_Ignored:
				dbg_puts("IGNR");
				break;

			case CXDiagnostic_Fatal:
				if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_KILL_ON_DIAGNOSTICS)
					h_ctx->m_err = ACLSPV_COMPILE_DIAG_FTAL;
				dbg_puts("FTAL");
				break;
			default:
				ae2f_unreachable();
		}

		clang_disposeString(TXT);
	}

	return aclspv_ctx_compile(h_ctx, clang_getTranslationUnitCursor(h_tu));
}

ae2f_extern ACLSPV_ABI_DECL aclspv_compile_t aclspv_ctx_compile2(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt,
		CXIndex							h_cxidx
		) {
	CXTranslationUnit	CXTU;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(h_cxidx)		return ACLSPV_COMPILE_ERR_CXIDX;
	ae2f_ifezerr(rdwr_unsaved)	return ACLSPV_COMPILE_PRM_INVAL_NIL;

	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	{
		enum CXErrorCode CXERR = clang_parseTranslationUnit2(
				h_cxidx, ae2f_NIL
				, (const char**)rd_argv_opt
				, c_argc
				, rdwr_unsaved, c_unsaved_count
				, CXTranslationUnit_None, &CXTU
				);

		if(rwr_cxerr_opt)	*rwr_cxerr_opt = CXERR;
	}

	ae2f_ifezerr(CXTU)		return h_ctx->m_err = ACLSPV_COMPILE_ERR_CXTU;
	ae2f_ifeznrm(h_ctx->m_err)	h_ctx->m_err = aclspv_ctx_compile1(h_ctx, CXTU);
	clang_disposeTranslationUnit(CXTU);

	return h_ctx->m_err;
}

ae2f_extern ACLSPV_ABI_DECL aclspv_compile_t aclspv_ctx_compile3(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
		) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(rdwr_unsaved)	return ACLSPV_COMPILE_PRM_INVAL_NIL;

	{
		const CXIndex CXIDX		= clang_createIndex(0, 0);
		ae2f_ifezerr(CXIDX)		{ 
			clang_disposeIndex(CXIDX);
			return h_ctx->m_err = ACLSPV_COMPILE_ERR_CXIDX; 
		}
		h_ctx->m_err = aclspv_ctx_compile2(h_ctx, rdwr_unsaved, c_unsaved_count, rd_argv_opt, c_argc, rwr_cxerr_opt, CXIDX);
		clang_disposeIndex(CXIDX);
	}

	return h_ctx->m_err;
}
