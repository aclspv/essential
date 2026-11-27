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

#ifndef	aclexpr_beforevisit_cond_h
#define	aclexpr_beforevisit_cond_h

#include <aclexpr.h>

/** `c ? t : f` — impure-arm branch/phi lane.
 *
 * called for each operand cursor of a conditional operator; `c_opidx`
 * names the operand being entered:
 *
 *   0 = condition, 1 = true arm, 2 = false arm.
 *
 * at idx 1 it decides whether the arms are pure. `OpSelect` evaluates
 * both arms, which is wrong exactly when an arm has an observable side
 * effect, so either impure arm turns the expression into a branch/phi
 * lane. at idx 2 it closes the true-arm block and opens the false one.
 *
 * 1 for something wrong.
 * */
aclspv_bool_t aclexpr_beforevisit3_cond(
		CXCursor					c_cur,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		);

#endif
