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

#ifndef	aclutil_capability_h
#define aclutil_capability_h

#include <aclspv.h>
#include <aclctx/err.h>

/** 1 on success */
ae2f_inline_f static aclspv_bool_t	aclutil_add_capability_by_flag(h_aclspv_ctx_t h_ctx, enum SpvCapability_ c) {
	if(aclspv_chck_capability_from_ctx(h_ctx, c))		return 1;
	ae2f_ifnezerr(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_EXPLICIT_CAPABILITY) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_NOIMPL_CAPABILITY);
		return 0;
	}

	return (aclspv_bool_t)ae2f_expected_strict(aclspv_add_capability_from_ctx(h_ctx, c)); 
}

#endif
