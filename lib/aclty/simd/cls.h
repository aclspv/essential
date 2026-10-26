#ifndef	aclty_simd_cls_h
#define aclty_simd_cls_h

#include <util/iddef.h>
#include <aclty.h>

ae2f_inline_f static aclspv_bool_t aclty_is_simd_flt(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclid_is_float(aclty_simd_class((c), (h)));
}

ae2f_inline_f static aclspv_bool_t aclty_is_simd_int(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclid_is_int(aclty_simd_class((c), (h)));
}

ae2f_inline_f static aclspv_bool_t aclty_is_simd_uint(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclid_is_uint(aclty_simd_class((c), (h)));
}

ae2f_inline_f static aclspv_wrd_t aclty_get_simd_bit_width(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclid_get_bit_width(aclty_simd_class(c, h));
}

#endif
