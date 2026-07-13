#ifndef	aclattr_main_h
#define aclattr_main_h

#include <aclspv.h>
#include <ae2f/pat/dbg.h>

ae2f_unused static enum CXChildVisitResult s_aclattr_main(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		) {
	h_aclspv_ctx_t	ctx = h_ctx;
	ae2fpat_assertassume(ctx);

	return ctx->m_work_nxt ? ctx->m_work_nxt(
			h_cur
			, h_parent
			, ctx->m_work_data
			) : CXChildVisit_Break;
}

#endif
