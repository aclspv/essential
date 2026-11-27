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

#ifndef	aclcur_ssa_h
#define	aclcur_ssa_h

#include <aclspv.h>
#include "./lbl/id.h"

/** emit-liveness of a block, shared between the value-flow resolution
 * (load.c) and the variable elision (reg/var.c).
 *
 * the emission sweep powers off every label whose reference count
 * cascades to zero; a block whose instructions never land in the binary
 * cannot contribute stores, exit values, or references. the predicate
 * follows the predecessor edges up to the root (see load.c). */
aclspv_bool_t aclcur_ssa_lbl_live(aclcur_lblid_t c, h_aclspv_ctx_t h_ctx);

/** post-carve, pre-emit: complete the loop-header authoritative phi
 * chains (ACLCUR_FLAG0_PHI_AUTH) with the final-graph back-edge
 * carries (see load.c). runs between carve and emission. */
aclspv_compile_t aclcur_ssa_loop_carry_fill(h_aclspv_ctx_t h_ctx);

#endif

