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

#ifndef	aclspv_ctx_capability_h
#define aclspv_ctx_capability_h

#include "./common.h"
#include <spirv/unified1/spirv.h>

#include <aclspv/bool.h>
#include <aclspv/link.h>

#include <ae2f/cc/qualifier.h>
#include <ae2f/cc/fnpretty.h>
#include <ae2f/Cxx/extern.h>

/**
 * @brief	check if capacity `c_cap` is included in `rd_ctx`.
 * */
ae2f_extern ACLSPV_ABI_LINK ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_chck_capability_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const enum SpvCapability_		c_cap
		);

/**
 * @brief	add a capacity `c_cap` to `h_ctx`
 * 1 on success
 * */
ae2f_extern ACLSPV_ABI_LINK ae2f_noexcept aclspv_bool_t aclspv_add_capability_from_ctx(
		h_aclspv_ctx_t				h_ctx,
		const enum SpvCapability_		c_cap
		);

#endif
