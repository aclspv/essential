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

#ifndef	aclspv_ctx_dump_h
#define aclspv_ctx_dump_h

#include "./common.h"

#include <aclspv/enum/err.h>
#include <aclspv/link.h>

#include <ae2f/Cxx/extern.h>

/**
 * @brief	dump the spir-v output from `h_ctx` to `ir_output_opt`, with length to `r_output_count_opt`.
 * 		value of `ir_output_opt` must be manually freed by `free`.
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_dump_ctx_output(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const		r_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict	ir_output_opt
		);

/**
 * @brief	dump the spir-v output from `h_ctx` to `ret`.
 * @details	word count of `ret` is considered as a value of `rdwr_count_ret`.
 * 		when word count is not sufficient, value of `rdwr_count_ret` will be changed to 
 * 			actual required word count of `ret`.
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_dump_ctx_output1(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const	rdwr_count_ret,
		aclspv_wrd_t* ae2f_restrict const	ret
		);

#endif
