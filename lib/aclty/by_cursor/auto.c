#include "./std430.h"
#include "./std140.h"

#include "../by_cursor.h"
#include "../layout.h"

#include <aclattr/layout.h>

aclspv_id_t aclty_mk_by_cursor_dummy(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclty_layout_t layout = aclattr_get_layout(c_cursor, c_type, h_ctx);

	ae2f_ifnezerr(h_ctx->m_err)
		return 0;

	if(layout == ACLSPV_LAYOUT_MAX)
		layout = ACLSPV_LAYOUT_STD430;	/** default */

	switch(layout) {
		default:
		case ACLSPV_LAYOUT_MAX:
			return 0;

		case ACLSPV_LAYOUT_STD430:
			return aclty_mk_by_cursor_std430(c_cursor, c_type, h_ctx);

		case ACLSPV_LAYOUT_STD140:
			return aclty_mk_by_cursor_std140(c_cursor, c_type, h_ctx);
	}

	ae2f_unreachable();
}
