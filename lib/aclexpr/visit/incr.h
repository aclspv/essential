#ifndef	aclemit_expr_visit_incr_h
#define aclemit_expr_visit_incr_h

#include <clang-c/Index.h>
#include <util/cursor.h>
#include <util/emitx.h>
#include <util/constant.h>
#include <emit/expr.h>
#include <.aclspv/util/mkname.h>

ae2f_inline_f static enum EMIT_EXPR_ s_simple(
		const CXCursor			c_cur,
		const h_util_ctx_t		h_ctx,
		aclspv_id_t* ae2f_restrict const	wr_newid,
		aclspv_id_t* ae2f_restrict const	wr_type_fetched,
		util_literal* ae2f_restrict const	rwr_literal_opt
		) {

	switch((uint_leastenum_t)c_cur.kind) {
		default:
			return EMIT_EXPR_NOT_THE_CASE;

		case CXCursor_UnaryOperator:
			break;
	}

	return EMIT_EXPR_NOT_THE_CASE;
}
#endif
