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

#ifndef	aclutil_cxtp_fn_h
#define	aclutil_cxtp_fn_h

#include "./common.h"
#include "./fn/arg.h"
#include "./fn/build.h"
#include "./fn/dcr.h"

aclspv_id_t aclty_mk_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclty_mk_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		);

/** the same as aclty_mk_fn1, with an implicit leading `this` parameter when
 * c_cursor_compound is a method / constructor / destructor.
 *
 * `this` is a pointer to the semantic parent record in Function storage. a
 * static method is left exactly as aclty_mk_fn1. */
aclspv_id_t aclty_mk_fn1_this(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		);


aclty_t* aclty_cfg_fn(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx
		);

#endif
