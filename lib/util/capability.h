#ifndef	aclutil_capability_h
#define aclutil_capability_h

#include <aclspv.h>

ae2f_inline_f static aclspv_bool_t	aclutil_add_capability_by_flag(h_aclspv_ctx_t h_ctx, enum SpvCapability_ c) {
	if(aclspv_chck_capability_from_ctx(h_ctx, c))		return 1;
	ae2f_ifnezerr(h_ctx->m_flag0 & ACLSPV_CTX_FLAG0_EXPLICIT_CAPABILITY) {
		h_ctx->m_err = ACLSPV_COMPILE_NOIMPL_CAPABILITY;
		return 0;
	}

	return (aclspv_bool_t)ae2f_expected_strict(aclspv_add_capability_from_ctx(h_ctx, c));
}

#endif
