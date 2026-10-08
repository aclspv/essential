#ifndef	aclcur_ret_h
#define aclcur_ret_h

#include <aclspv.h>

/** TODO: type check should be done early with expression */
aclspv_id_t aclcur_cfg_ret(
		aclspv_id_t c_val_opt,
		h_aclspv_ctx_t h_ctx
		);

#include "./ret/emit.h"

#endif
