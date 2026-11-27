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

#ifndef	aclattr_ifpart_h
#define aclattr_ifpart_h

#include <aclspv.h>

struct s_attr_ifpart_t {
	CXCursor		m_parts[3];
	h_aclspv_ctx_t		m_ctx;
	aclspv_wrdcount_t	m_n; /** 0, 1, 2, 3 */ 
	aclspv_wrdcount_t	m_n_dummy; /** 0, 1, 2, 3 */ 
	aclspv_id_t		m_b_true;
	aclspv_id_t		m_b_false;
	aclspv_id_t		m_b_default;
};

/** @param wr_data: s_attr_ifpart */
static enum CXChildVisitResult s_attr_ifpart(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict wr_data) {
	(((struct s_attr_ifpart_t* ae2f_restrict)(wr_data)))->m_n++
		[((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_parts]
		= h_cur;

	(((struct s_attr_ifpart_t* ae2f_restrict)(wr_data)))->m_n_dummy++;

	if(((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_n == 3) {
		((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_b_false
			= ((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_ctx->m_id++;
	}

	return CXChildVisit_Continue;
}

#endif 
