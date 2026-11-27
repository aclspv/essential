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

#ifndef	aclexpr_ctor_h
#define	aclexpr_ctor_h

#include <acldef.h>
#include <aclty/common.h>

/** handle the initializer of one local variable.
 *
 *   S s;         no initialisation at all
 *   S s{};       explicit constructor call
 *   S s = {};    value initialisation (nil / zero)
 *   S s = other; value initialisation (copy the expression)
 *
 * returns 0 when c_init is not a constructor call (the caller keeps its
 * ordinary initializer path), 1 when it was handled (success or failure; the
 * context carries the error).
 *
 * the explicit constructor path uses the object's storage address as `this`,
 * so the object register is marked address-taken before the call. */
/** call a user-provided copy/move constructor into a fresh temporary and
 * return that temporary's value. 0 when no user-provided constructor exists
 * (the caller then keeps the plain value-copy path). */
aclspv_bool_t aclexpr_copy_ctor_value(
		CXCursor	c_call,
		aclspv_id_t*	wr_id,
		aclty_id_t*	wr_ty,
		h_aclspv_ctx_t	h_ctx
		);

aclspv_bool_t aclexpr_varinit(
		CXCursor	c_init,
		aclspv_id_t	c_obj,
		aclty_id_t	c_ty,
		aclspv_bool_t*	wr_constructed,
		h_aclspv_ctx_t	h_ctx
		);

#endif
