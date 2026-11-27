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

/** 
 * @file	simple1.h
 * @brief	int & float literals, variable references
 * */

#ifndef aclexpr_visit_val0_h
#define aclexpr_visit_val0_h

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <aclcur/common.h>
#include <aclcur/load/get.h>
#include <aclexpr/curreg.h>

#include <.aclspv/util/mkname.h>
#include <aclexpr/visit/api.h>

/** 
 * binary expression with 1 operators.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op0_val(
		const CXCursor			c_cur,
		h_aclspv_ctx_t				h_ctx,
		aclexpr_opbincfg_extra*		rdwr_extra,
		aclspv_id_t* ae2f_restrict const	wr_newid,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		);
#endif
