/** @file id.h @brief utility functions for id.h */

#ifndef	aclutil_iddef_h
#define aclutil_iddef_h



#include <aclspv.h>

/**
 * @brief type alias for `enum ID_DEFAULT_`
 * */
typedef uint_leastenum_t B_id_default_t;

ae2f_ccconst aclspv_bool_t aclid_is_castable(const B_id_default_t, const h_aclspv_ctx_t);
ae2f_ccconst aclspv_bool_t aclid_is_castable1(const B_id_default_t, const B_id_default_t, const h_aclspv_ctx_t);
ae2f_ccconst aclspv_bool_t aclid_is_uint(const B_id_default_t c_id_default);
ae2f_ccconst aclspv_bool_t aclid_is_sint(const B_id_default_t c_id_default);
ae2f_ccconst aclspv_bool_t aclid_is_signed(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_int(const B_id_default_t c_id_default);
ae2f_ccconst aclspv_bool_t aclid_is_float(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_default(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_number(const B_id_default_t c_id_default);

ae2f_ccconst B_id_default_t aclid_getty_float(unsigned bit_width);
ae2f_ccconst B_id_default_t aclid_getty_uint(unsigned bit_width);
ae2f_ccconst B_id_default_t aclid_getty_sint(unsigned bit_width);
ae2f_ccconst B_id_default_t aclid_number_cvtbitcount(const B_id_default_t c_id, const unsigned bitcount);

ae2f_ccconst unsigned aclid_get_bit_width(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t	aclid_is_vectorisable(const aclspv_id_t c_id);
aclspv_bool_t aclid_is_arithmetic(const aclspv_id_t c_id, const h_aclspv_ctx_t h_ctx);

aclspv_id_t aclid_mk_default_id(
		const B_id_default_t	c_id_default,
		h_aclspv_ctx_t	h_ctx
		);

#endif
