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
 * @file	extension.h
 * @author	dalmurii
 * */

#ifndef	aclutil_extension_h
#define aclutil_extension_h

#include <aclspv.h>
#include <aclctx/err.h>
#include <acldef.h>

/** 0 when failed */
ae2f_inline_f static aclspv_bool_t aclutil_add_extension_by_flag(
		x_aclspv_ctx* const ae2f_restrict	h_ctx,
		const char* ae2f_restrict const		c_ext) {
	if(aclspv_chck_extension_from_ctx(h_ctx, c_ext)) return 1;
	ae2f_ifnezerr(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_EXPLICIT_EXTENSION) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_NOIMPL_EXTENSION);
		return 0;
	}

	/** 0 when failed */
	return (aclspv_bool_t)ae2f_expected_strict(aclspv_add_extension_from_ctx(h_ctx, c_ext));
}

#endif
