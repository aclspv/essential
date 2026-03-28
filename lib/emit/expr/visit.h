#ifndef	aclexpr_visit_h
#define aclexpr_visit_h

#include <util/ctx.h>
#include "../expr.h"

ae2f_inline static enum EMIT_EXPR_ s_simple(
		const CXCursor			c_cur,
		const h_util_ctx_t		h_ctx,
		aclspv_id_t* ae2f_restrict const	wr_newid,
		aclspv_id_t* ae2f_restrict const	wr_type_fetched,
		util_literal* ae2f_restrict const	rwr_literal_opt
		);

ae2f_inline static enum EMIT_EXPR_ s_cast_numeric(
		const CXCursor				c_cur,
		const h_util_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale
		);

ae2f_inline static enum EMIT_EXPR_ s_basic_numeric_operation(
		const CXCursor				c_cur,
		const h_util_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale
		);

#endif
