#ifndef	aclutil_cxtp_fn_args_h
#define aclutil_cxtp_fn_args_h

#include "../common.h"

aclutil_cxtp* aclutil_cxtp_fn_get_arg(
		aclutil_cxtp* ae2f_restrict rdwr_fn,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

aclutil_cxtp* aclutil_cxtp_fn_add_arg(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx,
		aclutil_cxtp* ae2f_restrict rdwr_fn
		);
#endif
