#include "../lbl.h"
#include "../util.h"
#include "./dfs.h"

#include <aclutil/emitx.h>
#include <ae2f/cc.h>
#include <.aclspv/util/mkname.h>

static enum ACLCUR_LBL_DFS_ s_dec_ref(
		aclcur_t* ae2f_restrict h_cur,
		ae2f_unused aclcur_t* ae2f_restrict u0,
		void* ae2f_restrict p_id) {
	aclspv_id_t* ae2f_restrict p_id_real = p_id;

	if(*p_id_real == h_cur->m_id) {
		return ACLCUR_LBL_DFS_IN;
	}

	(void)--h_cur->m_num_ref;
	return  ACLCUR_LBL_DFS_CONTINUE;
}

aclspv_compile_t EMIT_aclcur_lbl(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	ae2f_assume(h);
	ae2f_assume(h->m_class == ACLCUR_LBL);
	ae2f_assume(h_ctx);

	/** decrease the reference count when powering this label off */
	unless(h->m_num_ref || (h->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_ROOT))) {
		aclspv_id_t rootid = h->m_id;
		(void)aclcur_lbl_dfs(
				rootid
				, s_dec_ref
				,&rootid
				, h_ctx
				);

		return ACLSPV_COMPILE_OK;
	}

	/**
	 * the root label heads the declaration stream: its OpLabel opens
	 * the entry block ahead of the Function-storage OpVariables, which
	 * emit into m_fndef as well (see var.c). the code stream, m_fnimpl,
	 * starts with the entry jump into the first statement block.
	 * */
	if(h->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_ROOT)) {
		ae2f_ifezerr(h_ctx->m_count.m_fndef = aclutil_emitx_2(
					&h_ctx->m_section.m_fndef
					, h_ctx->m_count.m_fndef
					, SpvOpLabel
					, h->m_id
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	} else {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpLabel
					, h->m_id
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	}

	return ACLSPV_COMPILE_OK;
}
