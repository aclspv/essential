#include <aclspv.h>

#include <util/log.h>

#include <aclemit/bodies.h>
#include <aclemit/export.h>
#include <aclsym/fn.h>
#include <aclctx/err.h>

#include <aclutil/capability.h>

/** addressing-model and memory-model capabilities, once per context. */
static aclspv_compile_t s_setup(h_aclspv_ctx_t h_ctx) {
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

	return h_ctx->m_err;
}

static void s_diagnostics(h_aclspv_ctx_t h_ctx, CXTranslationUnit h_tu) {
	unsigned CXTU_IDX_ERR;

	CXTU_IDX_ERR = clang_getNumDiagnostics(h_tu);

	if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS)
	while(CXTU_IDX_ERR-- && !h_ctx->m_err) {
		const CXDiagnostic DIAG = clang_getDiagnostic(h_tu, CXTU_IDX_ERR);
		enum CXDiagnosticSeverity SEVERITY = clang_getDiagnosticSeverity(DIAG);

		CXString TXT = clang_formatDiagnostic(
				DIAG
				, clang_defaultDiagnosticDisplayOptions()
				);

		dbg_puts(TXT.data);

		switch(SEVERITY) {
			case CXDiagnostic_Note:
				dbg_puts("NOTE");
				break;
			case CXDiagnostic_Error:
				if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_KILL_ON_DIAGNOSTICS)
					_aclctx_err(h_ctx, ACLSPV_COMPILE_DIAG_ERR);
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
					_aclctx_err(h_ctx, ACLSPV_COMPILE_DIAG_FTAL);
				dbg_puts("FTAL");
				break;
			default:
				ae2f_unreachable();
		}

		clang_disposeString(TXT);
	}
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile(
		h_aclspv_ctx_t	h_ctx,
		CXCursor	c_cur_root
		) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	/** the phases are one shot: compile once, then build once. */
	if(h_ctx->m_phase >= ACLSPV_CTX_PHASE_COMPILE)
		return h_ctx->m_err;

	ae2f_ifnezerr(s_setup(h_ctx))
		return h_ctx->m_err;

	/** pass 1: declarations before bodies. every file-scope function of
	 * the main file is cached with its function type and checked for a
	 * conflicting redeclaration. */
	aclctx_err_check2(h_ctx, aclsym_fn_gather(h_ctx, c_cur_root))
		return h_ctx->m_err;

	h_ctx->m_phase = ACLSPV_CTX_PHASE_COMPILE;

	/** pass 2, compile half: every body that can be emitted now, in
	 * dependency order. a body waiting on a body-less callee is the build
	 * phase's job. */
	ae2f_ifnezerr(aclemit_compile_bodies(h_ctx, c_cur_root))
		return h_ctx->m_err;

	/** Linkage is not requested here: the compile phase cannot know whether
	 * the module will have an entry point — one may still be deferred. the
	 * build phase decides, after every body is out. */

	return h_ctx->m_err;
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile1(
		h_aclspv_ctx_t		h_ctx,
		CXTranslationUnit	h_tu
		) {
	ae2f_ifezerr(h_ctx)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(h_tu)	return ACLSPV_COMPILE_ERR_CXTU;

	s_diagnostics(h_ctx, h_tu);

	aclctx_err_check(h_ctx)
		return h_ctx->m_err;

	return aclspv_ctx_compile(
			h_ctx
			, clang_getTranslationUnitCursor(h_tu)
			);
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build(
		h_aclspv_ctx_t	h_ctx,
		CXCursor	c_cur_root
		) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	if(h_ctx->m_phase >= ACLSPV_CTX_PHASE_BUILD)
		return h_ctx->m_err;

	/** a build without a compile: run the compile phase first. */
	ae2f_ifnezerr(aclspv_ctx_compile(h_ctx, c_cur_root))
		return h_ctx->m_err;

	/** the build half: the imports, then the deferred bodies in dependency
	 * order. */
	h_ctx->m_phase = ACLSPV_CTX_PHASE_BUILD;

	ae2f_ifnezerr(aclemit_build_bodies(h_ctx, c_cur_root))
		return h_ctx->m_err;

	/** a module without any entry point is only valid under Linkage, so ask
	 * for it now that every body is out. imports and exports request it
	 * themselves; this is the last, real reason to carry it. */
	if(!h_ctx->m_count.m_entp) {
		ae2f_ifezerr(aclutil_add_capability_by_flag(
					h_ctx, SpvCapabilityLinkage))
			return h_ctx->m_err;
	}

	/** every emitted body has an id now: decorate the exports. */
	return aclemit_export(h_ctx, c_cur_root);
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build1(
		h_aclspv_ctx_t		h_ctx,
		CXTranslationUnit	h_tu
		) {
	ae2f_ifezerr(h_ctx)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(h_tu)	return ACLSPV_COMPILE_ERR_CXTU;

	s_diagnostics(h_ctx, h_tu);

	aclctx_err_check(h_ctx)
		return h_ctx->m_err;

	return aclspv_ctx_build(
			h_ctx
			, clang_getTranslationUnitCursor(h_tu)
			);
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build2(
		h_aclspv_ctx_t	h_ctx,
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

	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	{
		enum CXErrorCode CXERR = clang_parseTranslationUnit2(
				h_cxidx, 0
				, (const char**)rd_argv_opt
				, c_argc
				, rdwr_unsaved, c_unsaved_count
				, CXTranslationUnit_None, &CXTU
				);

		if(rwr_cxerr_opt)	*rwr_cxerr_opt = CXERR;
	}

	ae2f_ifezerr(CXTU) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ERR_CXTU);
		return h_ctx->m_err;
	}
	ae2f_ifeznrm(h_ctx->m_err) {
		_aclctx_err(h_ctx, aclspv_ctx_build1(h_ctx, CXTU));
		_aclctx_err_log(h_ctx);
	}

	clang_disposeTranslationUnit(CXTU);
	return h_ctx->m_err;
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build3(
		h_aclspv_ctx_t	h_ctx,
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
			_aclctx_err(h_ctx, ACLSPV_COMPILE_ERR_CXIDX);
			return h_ctx->m_err;
		}
		_aclctx_err(h_ctx, aclspv_ctx_build2(h_ctx, rdwr_unsaved, c_unsaved_count, rd_argv_opt, c_argc, rwr_cxerr_opt, CXIDX));
		clang_disposeIndex(CXIDX);
	}

	return h_ctx->m_err;
}
