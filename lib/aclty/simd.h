/**
 * scalar-aggregate types:
 * 	using same operator as its element type is same. (aclexpr is handeling this)
 * 	constant to simd casting is allowed via broadcasting.
 * */

#ifndef	aclty_simd_h
#define aclty_simd_h

#include "./common.h"

/** 
 * when the type is simd's element type.
 * returns 0 when `c` is neither scalar nor simd type.
 *
 * this may not return scalar type. for that behaviour: see `aclty_simd_class`
 * */
aclspv_id_t aclty_simd_elmtype(aclspv_id_t c, h_aclspv_ctx_t h_ctx); 

/** 
 * returns c_sub_opt when c is made upon c_sub_opt.
 * returns c's scalar type when c_sub_opt is zero.
 * this breaks down recursively
 * */
static aclspv_id_t aclty_simd_elmtype_r(aclspv_id_t c, aclspv_id_t c_sub_opt, h_aclspv_ctx_t h_ctx) {
	while(c && c != c_sub_opt) {
		aclspv_id_t c_new = aclty_simd_elmtype(c, h_ctx);
		if(c == c_new)
			break;

		c = c_new;
	}

	return c == c_sub_opt ? c_sub_opt : c_sub_opt ? 0 : c;
}

/** 
 * when the type is simd's class.
 * this function guarantees to return scalar type when succeed.
 *
 * useful for check if ( A * A ) thing or bit-casting is possible.
 * */
ae2f_inline_f static aclspv_id_t aclty_simd_class(aclspv_id_t c, h_aclspv_ctx_t h_ctx) {
	return aclty_simd_elmtype_r(c, 0, h_ctx);
}

/**
 * returns the likely-element type.
 * for example when c0 is u64x4 and c1 is u64, this function should return c1.
 * does not work on other element types.
 * */
ae2f_inline_f static aclspv_id_t aclty_simd_get_subtype(aclspv_id_t c0, aclspv_id_t c1, h_aclspv_ctx_t h_ctx) {
	if(c0 == c1)
		return c0;

	if(aclty_simd_elmtype_r(c0, c1, h_ctx)) {
		return c1;
	}

	if(aclty_simd_elmtype_r(c1, c0, h_ctx)) {
		return c0;
	}

	return 0;
}

/** 
 * simd gate: returns non-zero when c is a scalar or vector type.
 * for scalars, aclty_simd_class returns c itself.
 * */
ae2f_inline_f static aclspv_bool_t aclty_is_simd(aclspv_id_t c, h_aclspv_ctx_t h_ctx) {
	return (aclspv_bool_t)!!(aclty_simd_class(c, h_ctx));
}

/** check if `c` is simd class, therefore scalar type. */
ae2f_inline_f ae2f_ccconst static aclspv_bool_t aclty_is_simd_class(aclspv_id_t c) {
	return c != ACLSPV_ID_VOID && c < ACLSPV_ID_END;
}


/**
 * returns a type with a class of `c_class` and structure as `c` has.
 * for a class, see `aclty_simd_class`.
 *
 * returns 0 when failed.
 * */
aclspv_id_t aclty_simd_chxg_cls(aclspv_id_t c, aclspv_id_t c_class, h_aclspv_ctx_t h);

#endif
