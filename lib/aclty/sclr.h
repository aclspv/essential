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
ae2f_ccconst aclty_sclr_width_t aclty_get_sclr_width(const aclty_sclr_t c);

/** check if `c` is scalar type. */
ae2f_inline_f ae2f_ccconst static aclspv_bool_t aclty_is_sclr(aclty_sclr_t c) {
	return ae2f_static_cast(aclspv_bool_t, c != (aclty_sclr_t)ACLSPV_ID_VOID && c < (aclty_sclr_t)ACLSPV_ID_END);
}

#endif
