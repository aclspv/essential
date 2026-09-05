#ifndef	aclutil_cxtp_fn_args_h
#define aclutil_cxtp_fn_args_h

#include "../common.h"

aclty_t* aclty_get_fn_arg(
		aclty_t* ae2f_restrict rdwr_fn,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

aclty_t* aclty_add_fn_arg(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx,
		aclty_t* ae2f_restrict rdwr_fn
		);
#endif
