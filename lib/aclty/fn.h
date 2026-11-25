#ifndef	aclutil_cxtp_fn_h
#define	aclutil_cxtp_fn_h

#include "./common.h"
#include "./fn/arg.h"
#include "./fn/build.h"
#include "./fn/dcr.h"

aclspv_id_t aclty_mk_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclty_mk_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		);

/** the same as aclty_mk_fn1, with an implicit leading `this` parameter when
 * c_cursor_compound is a method / constructor / destructor.
 *
 * `this` is a pointer to the semantic parent record in Function storage. a
 * static method is left exactly as aclty_mk_fn1. */
aclspv_id_t aclty_mk_fn1_this(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		);


aclty_t* aclty_cfg_fn(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx
		);

#endif
