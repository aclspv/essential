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

#ifndef	aclutil_cxtp_fn_args_h
#define aclutil_cxtp_fn_args_h

#include "../common.h"

aclty_t* aclty_get_fn_arg(
		aclty_t* ae2f_restrict rdwr_fn,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

aclty_t* aclty_add_fn_arg(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx,
		aclty_t* ae2f_restrict rdwr_fn
		);
#endif
