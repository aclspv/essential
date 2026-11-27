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

#ifndef	aclcur_class_h
#define aclcur_class_h

#include <aclspv.h>

enum ACLCUR_ {
	ACLCUR_RESERVED,
	/** SpvOpBranch */
	ACLCUR_J,
	/** label */
	ACLCUR_LBL,
	/** SpvOpBranchConditional */
	ACLCUR_JC,
	/** literal register. */
	ACLCUR_REG,
	ACLCUR_RET,
	/** load unpredicted register */
	ACLCUR_LOAD,
	ACLCUR_STORE,
	/** load from other branch */
	ACLCUR_PHI,
	ACLCUR_OP1,
	ACLCUR_OP2,
	ACLCUR_OP3,
	/** op with variable operand count (linked-list operand chain) */
	ACLCUR_OPX,
	/** operand-link node of an ACLCUR_OPX op. never emitted. */
	ACLCUR_OPX_MBR,
	/** result-less two-operand instruction (SpvOpStore): [op|3][o0][o1].
	 * no Result Type, no Result. emitted unconditionally — a store has
	 * side effects, so it is never dropped as unreferenced. */
	ACLCUR_ST,

	/** SpvOpSwitch: the terminator of a switch header block. carries the
	 * selector, the default target, the selection merge and the list of
	 * (literal, target) case entries. */
	ACLCUR_SW,

	/** case entry of an ACLCUR_SW node: literal value and target label.
	 * appendix after its SW node, never emitted. */
	ACLCUR_SW_CASE,

	/** SpvOpUnreachable: terminator of a block that is structurally
	 * emitted (referenced as a merge block) but has no live incoming
	 * edge. */
	ACLCUR_UNREACHABLE
};

/** `aclcur_t` is forward-declared in the public context header. */
typedef struct aclcur* ae2f_restrict h_aclcur_t;

#endif
