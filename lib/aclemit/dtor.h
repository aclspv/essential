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

#ifndef	aclemit_dtor_h
#define	aclemit_dtor_h

#include <acldef.h>
#include <aclty/common.h>

/** reset the destructor scope stack for one function body. */
void aclemit_dtor_begin(h_aclspv_ctx_t h_ctx);

/** discard the stack after the body is carved. */
void aclemit_dtor_end(h_aclspv_ctx_t h_ctx);

/** enter a lexical block. */
aclspv_bool_t aclemit_dtor_push(h_aclspv_ctx_t h_ctx);

/** leave the current lexical block, emitting its live destructors in reverse
 * declaration order. */
aclspv_bool_t aclemit_dtor_pop(h_aclspv_ctx_t h_ctx);

/** emit every live destructor and clear the reservations, without changing
 * the scope stack (a return / break / goto leaves through the frames). */
aclspv_bool_t aclemit_dtor_flush_all(h_aclspv_ctx_t h_ctx);

/** flush and remove every frame whose scale id is greater than c_base_id.
 * used by the inliner when an inlined body returns. */
aclspv_bool_t aclemit_dtor_flush_to(
		h_aclspv_ctx_t	h_ctx,
		aclspv_wrd_t	c_base_id
		);

/** reserve a destructor for the local declared by c_var in the current
 * block. does nothing when the type has no user-declared destructor. */
aclspv_bool_t aclemit_dtor_reserve(
		CXCursor	c_var,
		aclspv_id_t	c_reg,
		aclty_id_t	c_ty,
		h_aclspv_ctx_t	h_ctx
		);

#endif
