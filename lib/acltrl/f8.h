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

#ifndef	acltrl_f8_h
#define acltrl_f8_h

#include <aclspv.h>

/** `ae2f::num`'s generic float-to-float cast (used for the E5M2 encoding,
 * which is a plain IEEE-754 layout with a 5-bit exponent / 2-bit mantissa). */
#include <.ae2f/num/flt/cast.h>

typedef aclspv_wrd_t	acltrl_f8_t;

/** 2^c_exp as a float; c_exp must be inside the f32 normal exponent range. */
ae2f_unused ae2f_ccconst ae2f_inline_f static float acltrl_pow2i(const int c_exp) {
	union {
		aclspv_wrd_t	m_u32;
		float		m_f32;
	} CV;

	CV.m_u32 = (aclspv_wrd_t)((aclspv_wrd_t)(127 + c_exp) << 23);
	return CV.m_f32;
}

/** f32 value -> FP8 E5M2 bit pattern.
 *
 * E5M2 is IEEE-754 with 5 exponent bits (bias 15) and 2 mantissa bits: it has
 * an implicit leading bit and infinity. the mantissa is truncated.
 * */
ae2f_unused ae2f_ccconst ae2f_inline_f static acltrl_f8_t acltrl_f32_to_f8_e5m2(
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
			, aclspv_wrd_t,  2, 5, 0x0f, 0x1f, 1, 0
			, aclspv_wrd_t, 23, 8, 0x7f, 0xff, 1, 0
			, dst, src, idx, exp);

	return (acltrl_f8_t)(dst[0] & 0xFFu);
}

/** FP8 E5M2 bit pattern -> f32 value. */
ae2f_unused ae2f_ccconst ae2f_inline_f static float acltrl_f8_e5m2_to_f32(
		const acltrl_f8_t c_bits) {
	aclspv_wrd_t	src[1];
	aclspv_wrd_t	dst[1];
	aclspv_wrd_t	idx;
	aclspv_wrd_t	exp;
	union {
		float		m_flt;
		aclspv_wrd_t	m_u32;
	} CV;

	src[0] = (aclspv_wrd_t)(c_bits & 0xFFu);
	dst[0] = 0;

	_ae2fnum_flt_cast(
			aclspv_wrd_t
			, aclspv_wrd_t, 23, 8, 0x7f, 0xff, 1, 0
			, aclspv_wrd_t,  2, 5, 0x0f, 0x1f, 1, 0
			, dst, src, idx, exp);

	CV.m_u32 = dst[0];
	return CV.m_flt;
}

/** f32 value -> FP8 E4M3 bit pattern.
 *
 * E4M3 has no implicit leading bit and no infinity: exponent 15 with an
 * all-ones mantissa is NaN, and every other exponent-15 value is a finite
 * "normal" (the max is 448 = 0x7E). there is no generic IEEE cast for that
 * layout, so this is done by hand. out-of-range values saturate to the max
 * finite magnitude and f32 subnormals flush to zero. the mantissa is
 * truncated.
 * */
ae2f_unused ae2f_ccconst ae2f_inline_f static acltrl_f8_t acltrl_f32_to_f8_e4m3(
		const float c_val) {
	union {
		float		m_flt;
		aclspv_wrd_t	m_u32;
	} CV;
	aclspv_wrd_t	sign, e32, m32, sig, out;
	int		e;

	CV.m_flt = c_val;
	sign = (CV.m_u32 >> 31) & 1u;
	e32 = (CV.m_u32 >> 23) & 0xFFu;
	m32 = CV.m_u32 & 0x7FFFFFu;

	/** infinity / NaN -> NaN (0x7F). */
	if(e32 == 0xFFu) {
		out = (sign << 7) | 0x7Fu;
		return (acltrl_f8_t)out;
	}

	/** f32 zero and subnormals are below the E4M3 range: flush to zero. */
	if(e32 == 0u) {
		out = sign << 7;
		return (acltrl_f8_t)out;
	}

	e = (int)e32 - 127;
	sig = 0x800000u | m32;	/** 24-bit significand, 1.m32 */

	/** above the largest finite (448 = 1.75 * 2^8): saturate to it. */
	if(e > 8) {
		out = (sign << 7) | 0x7Eu;
		return (acltrl_f8_t)out;
	}

	if(e >= -6) {
		/** normal: E = e + 7 in [1, 14] (e == 8 gives E == 15). */
		const aclspv_wrd_t E = (aclspv_wrd_t)(e + 7);
		aclspv_wrd_t M = (sig >> 20) & 0x7u;

		/** E4M3 has no infinity: 0x7F/0xFF is NaN, so clamp it to 0x7E. */
		if(E == 15u && M == 7u)
			M = 6u;

		out = (sign << 7) | (E << 3) | M;
		return (acltrl_f8_t)out;
	}

	if(e >= -9) {
		/** subnormal (E == 0): value = M * 2^-9, M in [1, 7]. */
		const aclspv_wrd_t M = (sig >> (unsigned)(14 - e)) & 0x7u;
		out = (sign << 7) | M;
		return (acltrl_f8_t)out;
	}

	/** below the smallest subnormal: flush to zero. */
	out = sign << 7;
	return (acltrl_f8_t)out;
}

/** FP8 E4M3 bit pattern -> f32 value. */
ae2f_unused ae2f_ccconst ae2f_inline_f static float acltrl_f8_e4m3_to_f32(
		const acltrl_f8_t c_bits) {
	const aclspv_wrd_t B = c_bits & 0xFFu;
	const aclspv_wrd_t SIGN = (B >> 7) & 1u;
	const aclspv_wrd_t E = (B >> 3) & 0xFu;
	const aclspv_wrd_t M = B & 0x7u;
	float v;

	if(E == 0u) {
		/** subnormal: (M / 8) * 2^-6. */
		v = ((float)M / 8.0f) * acltrl_pow2i(-6);
	} else {
		/** normal: (1 + M / 8) * 2^(E - 7). */
		v = (1.0f + (float)M / 8.0f) * acltrl_pow2i((int)E - 7);
	}

	return SIGN ? -v : v;
}

#endif
