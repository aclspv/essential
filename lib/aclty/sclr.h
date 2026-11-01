/** @file id.h @brief utility functions for id.h */

#ifndef	aclty_sclr_h
#define aclty_sclr_h

#include "./default.h"
#include <acldef.h>

/** scalar id */
typedef aclty_default_t	aclty_sclr_t;

/** bit width of scalar-default type */
typedef aclspv_uiter_t	aclty_sclr_width_t;

ae2f_ccconst aclspv_bool_t aclty_is_sclr_uint(const aclty_sclr_t c);
ae2f_ccconst aclspv_bool_t aclty_is_sclr_sint(const aclty_sclr_t c);
ae2f_ccconst aclspv_bool_t aclty_is_sclr_signed(const aclty_sclr_t c);

ae2f_ccconst aclspv_bool_t aclty_is_sclr_int(const aclty_sclr_t c);
ae2f_ccconst aclspv_bool_t aclty_is_sclr_flt(const aclty_sclr_t c);
ae2f_ccconst aclspv_bool_t aclty_is_sclr_num(const aclty_sclr_t c);

ae2f_ccconst aclty_sclr_t aclty_get_sclr_flt(aclty_sclr_width_t c);
ae2f_ccconst aclty_sclr_t aclty_get_sclr_uint(aclty_sclr_width_t c);
ae2f_ccconst aclty_sclr_t aclty_get_sclr_sint(aclty_sclr_width_t c);
ae2f_ccconst aclty_sclr_t aclty_cvt_sclr_width(const aclty_sclr_t c, const aclty_sclr_width_t c_width);
ae2f_ccconst aclspv_uiter_t aclty_get_sclr_width(const aclty_sclr_width_t c);

/** check if `c` is scalar type. */
ae2f_inline_f ae2f_ccconst static aclspv_bool_t aclty_is_sclr(aclty_sclr_t c) {
	return c != ACLSPV_ID_VOID && c < ACLSPV_ID_END;
}

#endif
