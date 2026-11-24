#include "./export.h"
#include <aclctx/err.h>

#include <aclsym/fn.h>

#include <aclattr/export.h>

#include <aclutil/capability.h>
#include <aclutil/linkage.h>

#include <ae2f/pat/dbg.h>

static enum CXChildVisitResult s_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_ctx
		) {
	h_aclspv_ctx_t			ctx = rdwr_ctx;
	aclsym_fn_t* ae2f_restrict	p_sym;
	aclattr_export_t		EXP;
	aclspv_bool_t			c_export;
	aclspv_sz_t			c;

	if(c_cur.kind != CXCursor_FunctionDecl)
		return CXChildVisit_Continue;

	unless(clang_Location_isFromMainFile(clang_getCursorLocation(c_cur)))
		return CXChildVisit_Continue;

	/** entry points are not callable, so they have no symbol and no
	 * export: they are reached through OpEntryPoint. */
	p_sym = aclsym_fn_find(c_cur, ctx);

	unless(p_sym)
		return CXChildVisit_Continue;

	/** only an emitted definition has an OpFunction to decorate. */
	if(!(p_sym->m_flag0 & (aclspv_wrd_t)(
				aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)
				| aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))))
		return CXChildVisit_Continue;

	EXP = aclattr_get_export(c_cur);

	c_export = (aclspv_bool_t)(
			EXP.m_export
			|| (!EXP.m_hidden
				&& (ctx->m_usrcfg.m_flag0
					& ACLSPV_CTX_FLAG0_EXPORT_BY_DEFAULT)));

	unless(c_export)
		return CXChildVisit_Continue;

	ae2f_ifezerr(aclutil_add_capability_by_flag(ctx, SpvCapabilityLinkage))
		return CXChildVisit_Break;

	ae2f_ifezerr(c = aclutil_emit_linkage(
				&ctx->m_section.m_decorate
				, ctx->m_count.m_decorate
				, p_sym->m_id
				, aclsym_fn_name(p_sym, ctx)
				, (aclspv_wrd_t)SpvLinkageTypeExport)) {
		_aclctx_err(ctx, ACLSPV_COMPILE_ALLOC_FAILED);
		return CXChildVisit_Break;
	}

	ctx->m_count.m_decorate = c;

	return CXChildVisit_Continue;
}

aclspv_compile_t aclemit_export(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	clang_visitChildren(c_root, s_visit, h_ctx);

	return h_ctx->m_err;
}
