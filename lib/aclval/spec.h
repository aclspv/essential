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

#ifndef	aclval_spec_h
#define aclval_spec_h

#include <aclval.h>

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_i32(
		const aclspv_wrd_t c_key, const aclspv_wrd_t c_val, 
		h_aclspv_ctx_t h_ctx
		)
{
	return 
		aclval_cfg_spec_by_enum(
				c_key
				, c_val
				, h_ctx
				, ACLSPV_ID_I32
				).m_val;
}

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_i64(
		const aclspv_wrd_t c_key, 
		const aclspv_wrd2_t c_val, h_aclspv_ctx_t h_ctx) {
	return 
		aclval_cfg_spec_by_enum(
				c_key
				, c_val
				, h_ctx
				, ACLSPV_ID_I64
				).m_val;
}

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_f32(const aclspv_wrd_t c_key, const float c_val, h_aclspv_ctx_t h_ctx)
{
	union { aclspv_wrd_t m_u32; float m_f32; } U;
	U.m_f32 = c_val;
	return
		aclval_cfg_spec_by_enum(
				c_key
				, U.m_u32
				, h_ctx
				, ACLSPV_ID_F32
				).m_val;
}

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_f64(
		const aclspv_wrd_t c_key, 
		const double c_val, h_aclspv_ctx_t h_ctx) {
	union { u64_least m_u32; double m_f32; } U;
	U.m_f32 = c_val;
	return
		aclval_cfg_spec_by_enum(
				c_key
				, U.m_u32
				, h_ctx
				, ACLSPV_ID_F64
				).m_val;
}

#endif
