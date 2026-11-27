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

#ifndef	aclcur_op_none_h
#define	aclcur_op_none_h

#include <aclcur.h>
#include <aclctx/id.h>

#include <aclspv.h>

/**
 * construction of the instructions that carry NO Result Type and NO Result:
 *
 *   SpvOpControlBarrier  [op|4][execution scope][memory scope][semantics]
 *   SpvOpMemoryBarrier   [op|3][memory scope][semantics]
 *
 * this is the ACLCUR_OPX shape (a linked operand chain, so the words are
 * appended in program order and emitted later like every other instruction)
 * with a zero type: EMIT_aclcur_opx writes a type and a result id only for a
 * value node, and these instructions have neither.
 *
 * the words are scope / memory-semantics values, not ids. OpControlBarrier and
 * OpMemoryBarrier still take them as id operands, so `aclcur_cfg_barrier`
 * folds each one into an OpConstant of the builtin's own u32 parameter type.
 * */

/** @return the new node id, or 0 on failure. */
aclcur_id_t aclcur_cfg_barrier(
		enum SpvOp_	c_op,
		const aclspv_wrd_t	c_words[],
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		);

#endif
