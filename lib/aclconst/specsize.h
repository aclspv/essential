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

#ifndef	aclconst_specsize_h
#define aclconst_specsize_h

#include <aclspv.h>

typedef struct {
	aclspv_id_t	m_id;

	union {
		aclspv_wrd_t	m_wrd;
		aclspv_wrd2_t	m_wrd2;
		aclspv_wrdmax_t	m_max;
	}		m_lit;
} aclconst_specsz;

aclspv_bool_t aclconst_is_specsz_same(aclconst_specsz c_x, aclconst_specsz c_y, h_aclspv_ctx_t h_ctx);

#endif
