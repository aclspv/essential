#ifndef	aclutil_cxtp_by_cursor_h
#define aclutil_cxtp_by_cursor_h

#include "./common.h"

/** Returns the type id of type */
aclspv_id_t aclutil_mk_cxtp_by_cursor(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		);

#endif
