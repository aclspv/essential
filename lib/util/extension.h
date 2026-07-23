/**
 * @file	extension.h
 * @author	dalmurii
 * */

#ifndef	aclutil_extension_h
#define aclutil_extension_h

#include <aclspv.h>

ae2f_inline_f static aclspv_bool_t aclutil_add_extension_by_flag(
		x_aclspv_ctx* const ae2f_restrict	h_ctx,
		const char* ae2f_restrict const		c_ext) {
	if(aclspv_chck_extension_from_ctx(h_ctx, c_ext)) return 1;
	ae2f_ifnezerr(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_EXPLICIT_EXTENSION) {
		h_ctx->m_err = ACLSPV_COMPILE_NOIMPL_EXTENSION;
		return 0;
	}

	return (aclspv_bool_t)ae2f_expected_strict(aclspv_add_extension_from_ctx(h_ctx, c_ext));
}

#endif
