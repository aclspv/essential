#include "./root.h"
#include "./fndecl/norm.h"
#include "./fndecl/entp.h"

#include <aclspv.h>
#include <aclcur/common.h>

#include <ae2f/pat/dbg.h>
#include <aclattr/execmodel.h>

ae2f_inline_f static aclspv_wrd_t s_is_kernel(const CXCursor h_fndecl) {
	enum SpvExecutionModel_ MODEL = SpvExecutionModelMax;
	clang_visitChildren(h_fndecl, s_attr_execmodel, &MODEL);
	return (aclspv_wrd_t)(MODEL ^ SpvExecutionModelMax);
}

ae2f_extern enum CXChildVisitResult aclemit_root(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		) {
#define CTX	((h_aclspv_ctx_t)h_ctx)

	ae2fpat_assertassume(h_ctx);

	/** search */
	unless(h_cur.kind == CXCursor_CompoundStmt && h_parent.kind == CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	/** per-function aclcur bootstrap: the root label occupies node 0 and
	 * becomes the current label for the argument + body walk. */
	CTX->m_num_cursor = 0;

	ae2f_ifezerr(aclcur_cfg_root(ACLSPV_ID_VOID, CTX)) {
		CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
		return CXChildVisit_Break;
	}

	return (CTX->m_work_nxt
			= s_is_kernel(h_parent)
			? aclemit_fndecl_entp
			: aclemit_fndecl_norm
	       )(h_cur, h_parent, CTX->m_work_data);

#undef	CTX
}
