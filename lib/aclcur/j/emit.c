#include "./emit.h"
#include "../common.h"

#include <aclutil/emitx.h>

aclspv_compile_t EMIT_aclcur_jmp(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
#if	__ACLSPV_CHECK_EXTRA
	ae2f_ifezerr(h)				return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifezerr(h->m_class == ACLCUR_J)	return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifezerr(h_ctx)			return ACLSPV_COMPILE_UNHANDLED;
#else
	ae2f_assume(h);
	ae2f_assume_strict(h->m_class == ACLCUR_J);
	ae2f_assume(h_ctx);
#endif

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_2(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, SpvOpBranch
				, h->m_d.m_j.m_lbl
				))
		return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

#if	__ACLSPV_CHECK_EXTRA
	{
		ae2f_ifezerr(aclcur_get_node(h->m_d.m_j.m_lbl, h_ctx))
			return ACLSPV_COMPILE_UNHANDLED;
	}
#endif

	return ACLSPV_COMPILE_OK;
}
