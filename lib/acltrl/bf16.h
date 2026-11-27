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

#ifndef	acltrl_bf16_h
#define acltrl_bf16_h

#include <aclspv.h>

/** `ae2f::num`'s generic float-to-float cast. */
#include <.ae2f/num/flt/cast.h>

typedef aclspv_wrd_t acltrl_bf16_t;

ae2f_unused ae2f_ccconst ae2f_inline_f static acltrl_bf16_t acltrl_f32_to_bf16(
		const float c_val) {
	aclspv_wrd_t	src[1];
	aclspv_wrd_t	dst[1];
	aclspv_wrd_t	idx;
	aclspv_wrd_t	exp;
	union {
		float		m_flt;
		aclspv_wrd_t	m_u32;
	} CV;

	CV.m_flt = c_val;
	src[0] = CV.m_u32;
	dst[0] = 0;

	_ae2fnum_flt_cast(
			aclspv_wrd_t
			, aclspv_wrd_t,  7, 8, 0x7f, 0xff, 1, 0
			, aclspv_wrd_t, 23, 8, 0x7f, 0xff, 1, 0
			, dst, src, idx, exp);

	return (aclspv_wrd_t)((dst[0]) & 0xFFFFu);
}

ae2f_unused ae2f_ccconst ae2f_inline_f static float acltrl_bf16_to_f32(
		const acltrl_bf16_t c_bits) {
	aclspv_wrd_t	src[1];
	aclspv_wrd_t	dst[1];
	aclspv_wrd_t	idx;
	aclspv_wrd_t	exp;

	union {
		float		m_flt;
		aclspv_wrd_t	m_u32;
	} CV;

	src[0] = (aclspv_wrd_t)((c_bits & 0xFFFFu));
	dst[0] = 0;

	_ae2fnum_flt_cast(
			aclspv_wrd_t
			, aclspv_wrd_t, 23, 8, 0x7f, 0xff, 1, 0
			, aclspv_wrd_t,  7, 8, 0x7f, 0xff, 1, 0
			, dst, src, idx, exp);

	CV.m_u32 = dst[0];
	return CV.m_flt;
}

#endif
