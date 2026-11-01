#ifndef	aclty_simd_cls_h
#define aclty_simd_cls_h

#include <aclty/simd.h>
#include <aclty/sclr.h>

ae2f_inline_f static aclspv_bool_t aclty_is_simd_flt(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_is_sclr_flt(aclty_get_simd_sclr((c), (h)));
}

ae2f_inline_f static aclspv_bool_t aclty_is_simd_int(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_is_sclr_int(aclty_get_simd_sclr((c), (h)));
}

ae2f_inline_f static aclspv_bool_t aclty_is_simd_uint(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_is_sclr_uint(aclty_get_simd_sclr((c), (h)));
}

ae2f_inline_f static aclty_sclr_width_t aclty_get_simd_sclr_width(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_get_sclr_width(aclty_get_simd_sclr(c, h));
}

#endif
