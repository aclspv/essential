#include "./emit.h"
#include "../common.h"

#include "../load/get.h"

#include <aclutil/emitx.h>

aclspv_compile_t EMIT_aclcur_jc(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	ae2f_assume_strict(h->m_class == ACLCUR_JC);
	ae2f_assume(h);
	ae2f_assume(h_ctx);

	/** merge must immediately precede the conditional branch. */
	if(h->m_d.m_jc.m_merge) {
		if(h->m_d.m_jc.m_continue) {
			ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_4(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpLoopMerge
						, h->m_d.m_jc.m_merge
						, h->m_d.m_jc.m_continue
						, h->m_d.m_jc.m_ctl
						))
				return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
		} else {
			ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_3(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpSelectionMerge
						, h->m_d.m_jc.m_merge
						, h->m_d.m_jc.m_ctl
						))
				return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
		}
	}

	if(h->m_d.m_jc.m_brw_true + h->m_d.m_jc.m_brw_false) {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_6(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpBranchConditional
					, h->m_d.m_jc.m_cond
					, h->m_d.m_jc.m_lbl_true
					, h->m_d.m_jc.m_lbl_false
					, h->m_d.m_jc.m_brw_true
					, h->m_d.m_jc.m_brw_false
					))
			return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	} else {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_4(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpBranchConditional
					, h->m_d.m_jc.m_cond
					, h->m_d.m_jc.m_lbl_true
					, h->m_d.m_jc.m_lbl_false
					))
			return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	}

#if	__ACLSPV_CHECK_EXTRA
	{
		ae2f_ifezerr(aclcur_get_node(h->m_d.m_jc.m_lbl_true, h_ctx))
			return ACLSPV_COMPILE_UNHANDLED;
		ae2f_ifezerr(aclcur_get_node(h->m_d.m_jc.m_lbl_false, h_ctx))
			return ACLSPV_COMPILE_UNHANDLED;
		{
			aclcur_t b_cond = aclcur_get_load(h->m_d.m_jc.m_cond, h_ctx);

			ae2f_ifezerr(b_cond.m_val)
				return ACLSPV_COMPILE_UNHANDLED;
			ae2f_ifezerr(b_cond.m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE))
				return ACLSPV_COMPILE_UNHANDLED;
		}

		if(h->m_d.m_jc.m_merge) {
			ae2f_ifezerr(aclcur_get_node(h->m_d.m_jc.m_merge, h_ctx))
				return ACLSPV_COMPILE_UNHANDLED;
		}
		if(h->m_d.m_jc.m_continue) {
			ae2f_ifezerr(aclcur_get_node(h->m_d.m_jc.m_continue, h_ctx))
				return ACLSPV_COMPILE_UNHANDLED;
		}
	}
#endif

	return ACLSPV_COMPILE_OK;
}
