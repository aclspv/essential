#ifndef	aclemit_inline_h
#define	aclemit_inline_h

#include <acldef.h>
#include <aclty/common.h>

/** is this function's body shape supported by the inliner? multiple or
 * nested returns are not (they fall back to OpFunctionCall). */
aclspv_bool_t aclemit_inline_supported(CXCursor c_fn);

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

/** selection merge stack used by inline returns. */
aclspv_bool_t aclemit_inline_merge_push(h_aclspv_ctx_t h_ctx, aclspv_id_t c_merge);
void aclemit_inline_merge_pop(h_aclspv_ctx_t h_ctx);

/** at a selection merge, continue to the parent merge (or inline end) when a
 * return was taken in one of the arms. */
aclspv_bool_t aclemit_inline_gate(h_aclspv_ctx_t h_ctx);

/** the branch target for an inline return. */
aclspv_id_t aclemit_inline_return_target(h_aclspv_ctx_t h_ctx);
