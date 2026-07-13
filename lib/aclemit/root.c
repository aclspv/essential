#include "./root.h"
#include "./fndecl/norm.h"
#include "./fndecl/entp.h"

#include <util/is_kernel.h>
#include <util/fnctx.h>

#include <ae2f/pat/dbg.h>

ae2f_extern enum CXChildVisitResult aclemit_root(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		) {
#define CTX	((h_aclspv_ctx_t)h_ctx)

	ae2fpat_assertassume(h_ctx);

	/** search */
	unless(h_cur.kind == CXCursor_CompoundStmt && h_parent.kind == CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	return (CTX->m_work_nxt
			= s_aclutil_is_kernel(h_parent)
			? aclemit_fndecl_entp
			: aclemit_fndecl_norm
	       )(h_cur, h_parent, CTX->m_work_data);

#undef	CTX
}
