#ifndef aclutil_cxtp_by_cursor_h
#define aclutil_cxtp_by_cursor_h

#include "./common.h"
#include "./layout.h"

typedef aclspv_id_t aclty_mk_by_cursor_t(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx);

typedef aclty_layout_interface_t aclty_mk_by_cursor_info_t;

/* Returns the type id of type */
aclspv_id_t aclty_mk_by_cursor(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx,
		const aclty_mk_by_cursor_info_t* ae2f_restrict	rd_info
		);

aclspv_id_t aclty_mk_by_cursor_dummy(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		); 


#endif
