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

#ifndef	aclcur_emit_h
#define aclcur_emit_h

#include <aclspv.h>

/** 
 * flush the whole cursor buffer of the current function into emission.
 *
 * walks the physical cursor buffer in order and dispatches every node
 * to its emitter:
 *
 * - LBL	→ EMIT_aclcur_lbl (a dead label — m_num_ref == 0, not
 *			  ROOT — is pruned together with its whole block),
 * - J / JC / RET / OP1 / OP2 / OP3 → their EMIT_aclcur_* counterparts,
 * - REG	→ the register's m_emit_reg; a register without one
 *			  (pure alias, ALIAS2 member view, ...) owns no
 *			  storage of its own and is skipped,
 * - LOAD	→ the owning register's m_emit_load,
 * - STORE	→ the owning register's m_emit_store,
 * - RESERVED / PHI → never emitted by themselves: RESERVED is a reuse
 *			  slot, PHI is described by its owning LOAD node.
 */
aclspv_compile_t aclcur_emit(h_aclspv_ctx_t h_ctx);

#endif

