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

#ifndef	aclspv_ctx_ext_h

#include <aclspv/bool.h>
#include <aclspv/link.h>
#include <aclspv/common.h>
#include <aclspv/ctx/common.h>

#include <ae2f/Cxx/extern.h>
#include <ae2f/cc/fnpretty.h>

ae2f_extern ACLSPV_ABI_LINK ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_chck_extension_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const char* ae2f_restrict const		c_ext
		);

ae2f_extern ACLSPV_ABI_LINK ae2f_noexcept aclspv_bool_t aclspv_add_extension_from_ctx(
		const h_aclspv_ctx_t			rd_ctx,
		const char* ae2f_restrict const		c_ext
		);

#endif
