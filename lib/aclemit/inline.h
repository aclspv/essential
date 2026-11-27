/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#ifndef	aclemit_inline_h
#define	aclemit_inline_h

#include <acldef.h>
#include <aclty/common.h>
#include <acltrl.h>

/** is this function's body shape supported by the inliner?
 *
 * multiple returns through if/switch are inlined through a single-iteration
 * loop scaffold; returns under a nested loop are not a structured exit from
 * that loop, so such bodies fall back to OpFunctionCall. */
aclspv_bool_t aclemit_inline_supported(CXCursor c_fn);

/** inline one function body into the current control-flow context.
 *
 * c_args are already-converted parameter values, in function-type order
 * (including `this` for a method).
 *
 * a body whose only return is the last statement of its block is carved
 * straight into the caller's block: the return stores the result and leaves
 * the block open, so no scaffold, no carry phi — which is what keeps a literal
 * argument a literal on the way in AND on the way out.
 *
 * every other shape (several returns, or a return under a selection) is
 * wrapped in a single-iteration loop; its return paths branch to the loop
 * continue target and the result / done flag are carried by loop-header phis,
 * so the caller's own return state is untouched.
 *
 * @return 1 on success. wr_ret receives the loaded return value (0 for void).
 * wr_lit_mask_opt / wr_literal_opt receive that value's literal state: a
 * literal return value stays a literal here instead of collapsing into the
 * runtime phi, so a call site can keep folding (`f(4) + 1`).
 * */
aclspv_bool_t aclemit_inline_fn(
		CXCursor	c_fn,
		CXCursor	c_body,
		const aclspv_id_t* c_args,
		aclspv_wrd_t	c_num,
		aclspv_id_t*	wr_ret,
		acltrl_mask_t*	wr_lit_mask_opt,
		acltrl_t*	wr_literal_opt,
		h_aclspv_ctx_t	h_ctx
		);

/** the declaration whose body is being carved right now (a null cursor
 * outside a body). a call site names the CALLEE's parameter cursors, and so
 * does an inlined body: a pointer argument binds to these. */
CXCursor aclemit_inline_callee(h_aclspv_ctx_t h_ctx);

/** is an inline slice (the single-iteration loop scaffold of an inlined
 * body with returns) currently being carved? */
aclspv_bool_t aclemit_inline_nested(const x_aclspv_ctx* ae2f_restrict h_ctx);

/** is c_reg LOCAL to an inline slice: declared in one, or written by a block
 * of one?
 *
 * such a register lives inside the slice's single-iteration loop, whose
 * continue edge jumps back to the slice header. nothing of it "flows out"
 * the way a straight-line assignment does — only the slice's own result
 * register does — so the value flow must not treat its last store as the
 * register's value outside the slice: that store is defined INSIDE a loop
 * and dominates neither that loop's back edge nor the code after it. */
aclspv_bool_t aclemit_inline_is_local(
		aclspv_id_t		c_reg,
		const x_aclspv_ctx* ae2f_restrict	h_ctx
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
