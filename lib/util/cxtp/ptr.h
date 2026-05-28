#ifndef	util_cxtp_ptr_h
#define	util_cxtp_ptr_h

#include "./common.h"

aclspv_id_t aclutil_mk_cxtp_ptr(
		const aclspv_id_t		c_type_elm,
		const enum SpvStorageClass_	c_storage_class,
		const h_aclspv_ctx_t		h_ctx
		);

#endif
