#ifndef	acltrl_f16_h
#define	acltrl_f16_h

#include <aclspv.h>

/** `ae2f::num`'s generic float-to-float cast. */
#include <.ae2f/num/flt/cast.h>

typedef aclspv_wrd_t acltrl_f16_t;

/** f32 value -> IEEE binary16 bit pattern.
 *
 * the mantissa is truncated: `ae2f::num` keeps the most significant bits and
 * drops the rest.
 *
 * a 16-bit destination is addressed as an offset into 32-bit words, so the
 * result sits in the high half of `dst[0]`.
 * */
ae2f_unused ae2f_ccconst ae2f_inline_f static acltrl_f16_t acltrl_f32_to_f16(
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
			, aclspv_wrd_t, 10, 5, 0x0f, 0x1f, 1, 0
			, aclspv_wrd_t, 23, 8, 0x7f, 0xff, 1, 0
			, dst, src, idx, exp);

	return (aclspv_wrd_t)((dst[0]) & 0xFFFFu);
}

/** IEEE binary16 bit pattern -> f32 value. */
ae2f_unused ae2f_ccconst ae2f_inline_f static float acltrl_f16_to_f32(
		const acltrl_f16_t c_bits) {
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
			, aclspv_wrd_t, 10, 5, 0x0f, 0x1f, 1, 0
			, dst, src, idx, exp);

	CV.m_u32 = dst[0];
	return CV.m_flt;
}

#endif
