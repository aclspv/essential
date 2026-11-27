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

#ifndef	aclutil_op_h
#define aclutil_op_h

#include <aclspv.h>

/** 
 * returns whether the operation order matters
 * */
ae2f_inline_f static ae2f_ccconst aclspv_bool_t aclutil_op_ordered(const aclspv_wrd_t c) {
	switch(c) {
		/** rest */
		default:
			return 1;

		case SpvOpIAdd:
		case SpvOpIMul:
		case SpvOpFAdd:
		case SpvOpFMul:
		case SpvOpBitwiseOr:
		case SpvOpBitwiseAnd:
		case SpvOpBitwiseXor:
		case SpvOpLogicalEqual:
		case SpvOpLogicalNotEqual:
			return 0;
	}
}

#endif
