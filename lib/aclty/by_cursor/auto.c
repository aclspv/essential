#include "./std430.h"
#include <aclctx/err.h>
#include "./std140.h"

#include "../by_cursor.h"
#include "../layout.h"

#include <aclattr/layout.h>

aclspv_id_t aclty_mk_by_cursor_default(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclty_layout_t layout = aclattr_get_layout(c_cursor, c_type, h_ctx);

	aclctx_err_check(h_ctx)
		return 0;

	if(layout == ACLSPV_LAYOUT_MAX)
		layout = ACLSPV_LAYOUT_STD430;	/** default */

	/*** defualt layout is std430 */
	switch(layout) {
		default:
		case ACLSPV_LAYOUT_MAX:
			ae2f_unreachable();

		case ACLSPV_LAYOUT_STD430:
			return aclty_mk_by_cursor_std430(c_cursor, c_type, h_ctx);

		case ACLSPV_LAYOUT_STD140:
			return aclty_mk_by_cursor_std140(c_cursor, c_type, h_ctx);
	}

	ae2f_unreachable();
}
