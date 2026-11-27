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

#ifndef	aclemit_sync_h
#define	aclemit_sync_h

#include <aclspv.h>

/**
 * @file
 * @brief	the synchronisation builtins, lowered in place.
 *
 *	void aclspv_ctlbar(u32 scope_exec, u32 scope_mem, u32 mem_sem);  OpControlBarrier
 *	void aclspv_membar(u32 scope_mem, u32 mem_sem);                  OpMemoryBarrier
 *
 * both are `void` calls whose arguments are constants, and both have no
 * definition anywhere: the call is recognised by spelling and the
 * instruction is written into the block being carved. no symbol is created,
 * no body is looked for, and no import is declared even when the source
 * declared a prototype.
 *
 * a barrier is therefore a STATEMENT, not an expression: it has no result,
 * and every other expression lane is built around producing one. the body
 * emitter lowers it before it would evaluate the call as an expression.
 *
 * the scope / semantics words belong to the memory model the context
 * declares (GLSL450 numbers `SpvScope*` / `SpvMemorySemantics*Mask`, Vulkan
 * numbers `SpvScope*KHR`); the caller spells the number.
 * */

/** true when the call cursor spells a barrier builtin.
 *
 * the spelling is the whole test: a prototype for a barrier declares nothing
 * this module can call, so it must neither hide the barrier nor turn it into
 * an import.
 * */
aclspv_bool_t aclemit_sync_is_barrier(CXCursor c_cur);

/** lower one barrier call statement into the current block.
 *
 * the arguments are read off the cursor: each one must fold to an integer
 * constant that fits a word, because neither instruction takes a value.
 *
 * @return 0 on success; nonzero when the call is malformed (h_ctx->m_err set).
 * */
aclspv_bool_t aclemit_sync_barrier(CXCursor c_cur, h_aclspv_ctx_t h_ctx);

#endif
