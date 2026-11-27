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

#ifndef	acltrl_common_h
#define acltrl_common_h

#include <aclspv.h>

enum ACLTRL_MASK_ {
	ACLTRL_MASK_RT,
	ACLTRL_MASK_CONST	= 0x1,
	ACLTRL_MASK_LTRL	= 0x3
};

enum ACLTRL_ {
	ACLTRL_RT,
	ACLTRL_CONST,
	ACLTRL_LTRL
};

typedef aclspv_wrd_t	acltrl_mask_t;

typedef union {
	int		m_api_int;

	aclspv_swrd2_t	m_api_intmax;
	aclspv_wrd2_t	m_api_uintmax;

	aclspv_wrd2_t	m_u64;

	double		m_dbl;
	float		m_flt;

	aclspv_wrd_t	m_wrd;
	aclspv_wrd2_t	m_wrd2;
} acltrl_t;

struct acltrl_eval {
	acltrl_t	m_ret;
	aclspv_bool_t	m_found;
};

#endif
