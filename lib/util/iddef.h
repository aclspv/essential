/** @file id.h @brief utility functions for id.h */

#ifndef	aclutil_iddef_h
#define aclutil_iddef_h


#include "./id.h"
#include "./ctx.h"

/**
 * @brief type alias for `enum ID_DEFAULT_`
 * */
typedef uint_leastenum_t B_id_default_t;


ae2f_ccconst aclspv_bool_t aclid_is_unsigned(const B_id_default_t c_id_default);
ae2f_ccconst aclspv_bool_t aclid_is_signed(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_int(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_float(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_default(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_number(const B_id_default_t c_id_default);

ae2f_ccconst B_id_default_t aclid_getty_float(unsigned bit_width);

ae2f_ccconst B_id_default_t aclid_getty_unsigned(unsigned bit_width);
ae2f_ccconst B_id_default_t aclid_getty_signed(unsigned bit_width);

ae2f_ccconst unsigned aclid_get_bit_width(const B_id_default_t c_id_default);

ae2f_ccconst aclspv_bool_t aclid_is_arithmetic(const aclspv_id_t c_id);

aclspv_id_t aclid_mk_default_id(
		const B_id_default_t	c_id_default,
		h_util_ctx_t	h_ctx
		);


ae2f_ccconst B_id_default_t aclid_decide_cast(const e_id_default c_id0, const e_id_default c_id1);

#endif
