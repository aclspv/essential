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

#ifndef	aclexpr_aftervisit_h
#define	aclexpr_aftervisit_h

#include <aclexpr.h>

/** 1 for something wrong.
 *
 * `c_cur` is the cursor being entered. for a binary operator's second
 * operand this is the RHS cursor, which the short-circuit lane needs in
 * order to decide whether the RHS is free of side effects.
 * */
aclspv_bool_t aclexpr_beforevisit(
		CXCursor					c_cur,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		);

#endif
