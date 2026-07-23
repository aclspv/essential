/** 
 * @file conf.h 
 * @brief bootstrap the compiler state machine 
 * */

#ifndef	aclspv_impl_conf_h
#define aclspv_impl_conf_h

#include <util/emitx.h>
#include <util/id.h>
#include <util/capability.h>
#include <util/extension.h>

#include <aclspv.h>
#include <ae2f/pat/dbg.h>

#define	wrd_caps	h_ctx->m_section.m_capability 
#define wrd_caps_count	h_ctx->m_count.m_capability

#define	wrd_ext		h_ctx->m_section.m_ext
#define	wrd_ext_count	h_ctx->m_count.m_ext

ae2f_inline_f static e_aclspv_compile_t aclspv_ctx_conf(
		const h_aclspv_ctx_t		h_ctx
		)
{
	aclspv_wrd_t	POS = 0;

	ae2fpat_assertassume(h_ctx);

	/*** Shader Capability **/
	ae2f_expected_but_else(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityShader))
		return ACLSPV_COMPILE_ALLOC_FAILED_CFG;

	unless(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_LOGICAL) {
		ae2f_expected_but_else(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityAddresses))
			return ACLSPV_COMPILE_ALLOC_FAILED_CFG;
	}

	unless(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_OGL) {
		ae2f_expected_but_else(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityVulkanMemoryModel))
			return ACLSPV_COMPILE_ALLOC_FAILED_CFG;

		POS = wrd_ext_count;
		unless(wrd_ext_count = emit_opcode(&wrd_ext, wrd_ext_count, SpvOpExtension, 0))
			return ACLSPV_COMPILE_ALLOC_FAILED_CFG;
		unless(wrd_ext_count = aclutil_emit_str(&wrd_ext, wrd_ext_count
					, "SPV_KHR_vulkan_memory_model"))
			return ACLSPV_COMPILE_ALLOC_FAILED_CFG;
		set_oprnd_count_for_opcode(aclutil_get_wrd_of_vec(&wrd_ext)[POS], (aclspv_wrd_t)(wrd_ext_count - POS - 1));
	}

	/*** Extension Default **/
	ae2f_ifezerr(aclutil_add_extension_by_flag(h_ctx, "SPV_KHR_storage_buffer_storage_class"))
		return h_ctx->m_err ? h_ctx->m_err : ACLSPV_COMPILE_UNHANDLED;


	/*** Memory Model **/
	ae2f_expected_but_else(h_ctx->m_count.m_memmodel = aclutil_emitx_3(
				&h_ctx->m_section.m_memmodel
				, 0
				, SpvOpMemoryModel
				, h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_LOGICAL ? SpvAddressingModelLogical
				: h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_BUF64 ? SpvAddressingModelPhysical64 : SpvAddressingModelPhysical32 
				, h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_OGL ? SpvMemoryModelGLSL450 : SpvMemoryModelVulkan))
		return ACLSPV_COMPILE_ALLOC_FAILED_CFG;

	return ACLSPV_COMPILE_OK;
}


#endif
