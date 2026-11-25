#ifndef	aclemit_inline_h
#define	aclemit_inline_h

#include <acldef.h>
#include <aclty/common.h>

/** inline one function body into the current control-flow context.
 *
 * c_args are already-converted parameter values, in function-type order
 * (including `this` for a method). the body is carved into the caller graph;
 * its return paths branch to a local end label, so the caller's own return
 * state is untouched.
 *
 * @return 1 on success. wr_ret receives the loaded return value (0 for void).
 * */
aclspv_bool_t aclemit_inline_fn(
		CXCursor	c_fn,
		CXCursor	c_body,
		const aclspv_id_t* c_args,
		aclspv_wrd_t	c_num,
		aclspv_id_t*	wr_ret,
		h_aclspv_ctx_t	h_ctx
		);

#endif
