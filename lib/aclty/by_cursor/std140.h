#ifndef	aclty_by_cursor_std140_h
#define aclty_by_cursor_std140_h

#include "../by_cursor.h"

static aclspv_id_t aclty_mk_by_cursor_std140(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx) {
	aclty_mk_by_cursor_info_t a;
	a.m_add_struct_mbr	= aclty_add_struct_mbr_std140;
	a.m_array_stride	= 16;
	return aclty_mk_by_cursor(
			c_cursor
			, c_type
			, h_ctx
			, &a);
}

#endif
