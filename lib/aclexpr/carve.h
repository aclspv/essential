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

#ifndef	aclexpr_carve_h
#define	aclexpr_carve_h

#include <aclexpr/op.h>
#include <aclexpr.h>

/**
 * carve the expression whose scale is on top of the evaluator's stack.
 *
 * a lane's ACLEXPR_CARVE_NO reaches the context as an error here: a carve
 * that cannot be emitted must not let evaluation continue with a
 * half-carved expression, which is how a store could go missing from an
 * otherwise valid-looking module.
 *
 * @returns 0 when success
 * */
aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		);

#endif
