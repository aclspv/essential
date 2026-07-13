#include "./norm.h"
#include <util/fnctx.h>
#include <util/fn.h>
#include <util/cxtp.h>
#include <acldbg/mallocfree.h>
#include <.aclspv/vec.h>
#include <aclemit/fndecl/body.h>
#include <aclemit/root.h>

enum CXChildVisitResult aclemit_fndecl_norm(
		ae2f_unused CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData h_ctx
		) {
	h_aclspv_ctx_t ctx = h_ctx;
	aclutil_fn_t fn;

#if	1
	{
		ctx->m_work_nxt = aclemit_root;
		return CXChildVisit_Continue;
	}
#endif

	ctx->m_num_cursor	= 0;

	fn.m_fn		= h_parent;
	fn.m_id		= ctx->m_id++;
	fn.m_type_id	= aclutil_mk_cxtp_fn1(
			h_parent
			, clang_getCursorType(h_parent)
			, ctx
			);

	/** TODO: find if static or not and conditionally add */
	goto OUT;

OUT:
	_aclspv_grow_vec_with_copy(
			_aclspv_malloc
			, _aclspv_free
			, _aclspv_memcpy
			, L_new
			, ctx->m_fnlist.m_fn
			, (ctx->m_fnlist.m_num_fn + 1) * sizeof(fn));

	ae2f_ifezerr(ctx->m_fnlist.m_fn.m_p) {
		ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
		return CXChildVisit_Break;
	}

	((aclutil_fn_t* ae2f_restrict)ctx->m_fnlist.m_fn.m_p)
		[ctx->m_fnlist.m_num_fn++] = fn;

	{
		aclutil_fnctx	fnctx;
		fnctx.m_retid = ACLSPV_ID_VOID;
		return aclemit_fndecl_bodycfg(h_cur, &fnctx, h_ctx);
	}

}
