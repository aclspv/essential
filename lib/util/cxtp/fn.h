#ifndef	aclutil_cxtp_fn_h
#define	aclutil_cxtp_fn_h

#include "./common.h"

aclspv_id_t aclutil_mk_cxtp_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclutil_mk_cxtp_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		);

#endif
