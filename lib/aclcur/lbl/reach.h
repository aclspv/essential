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

#ifndef	aclcur_lbl_reach_h
#define aclcur_lbl_reach_h

#include "./id.h"

aclspv_bool_t aclcur_lbl_is_reachable(
		aclcur_lblid_t c_root,
		aclcur_lblid_t c_child,
		h_aclspv_ctx_t h_ctx
		);

/** unconditionally reachable */
aclspv_bool_t aclcur_lbl_is_reachable_uc(
		aclcur_lblid_t c_root,
		aclcur_lblid_t c_child,
		h_aclspv_ctx_t h_ctx);

/** true iff a successor of c_lbl can reach c_lbl back: the block
 * participates in a cycle (a loop block: header, body, or the continue
 * trampoline). */
aclspv_bool_t aclcur_lbl_in_cycle(
		aclcur_lblid_t c_lbl,
		h_aclspv_ctx_t h_ctx);

#endif
