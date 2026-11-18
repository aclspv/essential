#include "./imports.h"
#include "./sym.h"

#include <aclsym/fn.h>

#include <aclty.h>

#include <aclutil/capability.h>
#include <aclutil/emitx.h>
#include <aclutil/linkage.h>

#include <ae2f/pat/dbg.h>

/** emit the import declaration for one body-less symbol. */
static aclspv_compile_t s_declare(
		aclsym_fn_t*		p_sym,
		CXCursor		c_fn,
		x_aclspv_vec*		h_wrds,
		aclspv_wrdcount_t* ae2f_restrict	wr_count,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t			id_fn_ty;
	aclty_t* ae2f_restrict		p_fn_ty;
	aclspv_wrd_t			c_fnctrl;
	aclspv_sz_t			c;

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1(
				c_fn
				, clang_getCursorType(c_fn)
				, h_ctx))
		return h_ctx->m_err = (
				h_ctx->m_err
				? h_ctx->m_err
				: ACLSPV_COMPILE_UNHANDLED
				);

	ae2f_ifezerr(p_fn_ty = aclty_find(id_fn_ty, h_ctx))
		return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

	/** linkage between modules: the capability is what makes the
	 * LinkageAttributes decoration legal. */
	ae2f_ifezerr(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityLinkage))
		return h_ctx->m_err;

	p_sym->m_id = h_ctx->m_id++;

	/** name the symbol when the flag asks for it. */
	ae2f_ifnezerr(aclemit_fndecl_sym_name(p_sym, h_ctx))
		return h_ctx->m_err;

	/** a body-less function has no body to analyse: its declared promise
	 * (__attribute__((pure)) / const) is all there is, and the control mask
	 * is the only place the module can carry it. */
	c_fnctrl = 0;
	if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST))
		c_fnctrl |= (aclspv_wrd_t)SpvFunctionControlConstMask;
	if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE))
		c_fnctrl |= (aclspv_wrd_t)SpvFunctionControlPureMask;

	/** [result type][result id][control][function type], then no blocks:
	 * a declaration. the parameter types live in the OpTypeFunction. */
	c = aclutil_emitx_5(
			h_wrds
			, *wr_count
			, SpvOpFunction
			, p_fn_ty->m_info.m_fn.m_ret_id
			, p_sym->m_id
			, c_fnctrl
			, id_fn_ty
			);

	ae2f_ifezerr(c)
		return h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;

	c = emit_opcode(
			h_wrds
			, c
			, SpvOpFunctionEnd
			, 0
			);

	ae2f_ifezerr(c)
		return h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;

	*wr_count = c;

	/** OpDecorate %id LinkageAttributes "<name>" Import. */
	ae2f_ifezerr(c = aclutil_emit_linkage(
				&h_ctx->m_section.m_decorate
				, h_ctx->m_count.m_decorate
				, p_sym->m_id
				, aclsym_fn_name(p_sym, h_ctx)
				, (aclspv_wrd_t)SpvLinkageTypeImport))
		return h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;

	h_ctx->m_count.m_decorate = c;

	/** the callee now has an id in the module: call sites take the ordinary
	 * emitted path. */
	p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED);

	return ACLSPV_COMPILE_OK;
}

struct s_import_ctx {
	h_aclspv_ctx_t		m_ctx;
	x_aclspv_vec*		m_wrds;
	aclspv_sz_t*		m_count;
};

static enum CXChildVisitResult s_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_client
		) {
	struct s_import_ctx* ae2f_restrict	rdwr_ctx = rdwr_client;
	h_aclspv_ctx_t				ctx = rdwr_ctx->m_ctx;
	CXCursor			c_callee;
	aclsym_fn_t* ae2f_restrict	p_sym;

	if(c_cur.kind != CXCursor_CallExpr)
		return CXChildVisit_Recurse;

	c_callee = clang_getCursorReferenced(c_cur);

	if(clang_Cursor_isNull(c_callee) || c_callee.kind != CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	p_sym = aclsym_fn_find(c_callee, ctx);

	/** a builtin / header function, or a symbol we know: nothing to import. */
	unless(p_sym)
		return CXChildVisit_Recurse;

	/** a body in this translation unit is ordered by emission, not linked;
	 * an already declared one needs nothing. */
	if(p_sym->m_flag0 & (aclspv_wrd_t)(
				aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)
				| aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED)))
		return CXChildVisit_Recurse;

	ae2f_ifnezerr(s_declare(
				p_sym
				, c_callee
				, rdwr_ctx->m_wrds
				, rdwr_ctx->m_count
				, ctx))
		return CXChildVisit_Break;

	return CXChildVisit_Recurse;
}

aclspv_compile_t aclemit_fndecl_imports(
		CXCursor		c_body,
		x_aclspv_vec*		h_wrds,
		aclspv_wrdcount_t* ae2f_restrict	wr_count,
		h_aclspv_ctx_t		h_ctx
		) {
	struct s_import_ctx cf;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	ae2f_ifezerr(h_wrds)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(wr_count)		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	cf.m_ctx	= h_ctx;
	cf.m_wrds	= h_wrds;
	cf.m_count	= wr_count;

	clang_visitChildren(c_body, s_visit, &cf);

	return h_ctx->m_err;
}
