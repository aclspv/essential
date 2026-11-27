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

#ifndef aclattr_getkinder_h
#define aclattr_getkinder_h

#include <ae2f/pat/dbg.h>
#include <aclspv.h>

struct aclattr_getkinder {
	CXCursor* ae2f_restrict	m_cursor;
	enum CXChildVisitResult	m_res;

	aclspv_wrd_t		m_num_wanted;
	aclspv_wrd_t		m_num_grepped;
};

/**
 * @param	h_getkinder : aclattr_getkinder*
 * */
static enum CXChildVisitResult s_aclattr_getkinder_visit(
		CXCursor c_cur, 
		ae2f_unused CXCursor unused,
		CXClientData ae2f_restrict h_getkinder
		) {
	struct aclattr_getkinder* ae2f_restrict P_GETKINDER 
		= (struct aclattr_getkinder* ae2f_restrict)(h_getkinder);

	ae2fpat_assertassume(P_GETKINDER);
	ae2fpat_assertassume(P_GETKINDER->m_cursor);

	if(P_GETKINDER->m_num_wanted <= P_GETKINDER->m_num_grepped)
		return CXChildVisit_Break;


	P_GETKINDER->m_cursor[P_GETKINDER->m_num_grepped++]
		= c_cur;

	return P_GETKINDER->m_res;
}

#endif
