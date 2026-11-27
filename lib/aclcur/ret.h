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

#ifndef	aclcur_ret_h
#define aclcur_ret_h

#include <aclspv.h>

/** TODO: type check should be done early with expression */
aclspv_id_t aclcur_cfg_ret(
		aclspv_id_t c_val_opt,
		h_aclspv_ctx_t h_ctx
		);

/** SpvOpUnreachable terminator: a structurally emitted block with no live
 * incoming edge must still end with a branch/terminator instruction. */
aclspv_id_t aclcur_cfg_unreachable(h_aclspv_ctx_t h_ctx);

#include "./ret/emit.h"

#endif
