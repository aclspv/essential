/** 
 * @file conf.h 
 * @brief bootstrap the compiler state machine 
 * */

#ifndef	aclspv_impl_conf_h
#define aclspv_impl_conf_h

#include <util/id.h>

#include <aclutil/emitx.h>
#include <aclutil/capability.h>
#include <aclutil/extension.h>

#include <aclspv.h>
#include <ae2f/pat/dbg.h>

#define	wrd_caps	h_ctx->m_section.m_capability 
#define wrd_caps_count	h_ctx->m_count.m_capability

#define	wrd_ext		h_ctx->m_section.m_ext
#define	wrd_ext_count	h_ctx->m_count.m_ext

ae2f_inline_f static aclspv_compile_t aclspv_ctx_conf(
		const h_aclspv_ctx_t		h_ctx
		)
{
	ae2fpat_assertassume(h_ctx);

	/*** Memory Model **/
	ae2f_ifezerr(h_ctx->m_count.m_memmodel = aclutil_emitx_3(
				&h_ctx->m_section.m_memmodel
				, 0
				, SpvOpMemoryModel
				, h_ctx->m_usrcfg.m_mdl_addr 
				, h_ctx->m_usrcfg.m_mdl_mem
				))
		return ACLSPV_COMPILE_ALLOC_FAILED_CFG;

	return ACLSPV_COMPILE_OK;
}


#endif
