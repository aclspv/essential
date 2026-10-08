#ifndef	aclcur_util_h
#define aclcur_util_h

#include "./common.h"

ae2f_ccpure static ae2f_inline_f aclcur_t* s_get_buf(const x_aclspv_ctx* ae2f_restrict h_ctx) {
	return h_ctx->m_cursors.m_p;
}

ae2f_ccpure static ae2f_inline_f const aclcur_t* s_get_buf_c(const x_aclspv_ctx* ae2f_restrict h_ctx) {
	return h_ctx->m_cursors.m_p;
}

#endif
