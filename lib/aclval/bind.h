/*** descriptor set & binding index */
#ifndef	aclval_bind_h
#define aclval_bind_h

#include "../aclval.h"

/** 0xFFFFFFFF when failed. */
aclspv_wrd_t aclval_get_bind_last_index(aclspv_wrd_t c_desc, h_aclspv_ctx_t h);

/** 0xFFFFFFFF when failed. */
aclspv_wrd_t aclval_inc_bind_last_index(aclspv_wrd_t c_desc, h_aclspv_ctx_t h);

/** returns newer storage class */
aclspv_wrd_t aclval_cfg_bind_storage_class(
		aclspv_wrd_t c_desc,
		aclspv_wrd_t c_bind_idx,
		aclspv_wrd_t c_storage_class,
		h_aclspv_ctx_t h
		);

#endif
