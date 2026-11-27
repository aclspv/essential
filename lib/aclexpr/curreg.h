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

#ifndef	aclexpr_curreg_h
#define aclexpr_curreg_h

#include <aclcur/common.h>

/**
 * find the register node bound to a cursor.
 *
 * scans the cursor buffer for an ACLCUR_REG whose m_cur equals c_cur.
 * returns its id, or 0 when no such register exists yet.
 * */
static ae2f_inline_f aclspv_id_t aclexpr_curreg_find(CXCursor c_cur, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t i;
	aclcur_t* ae2f_restrict p = (aclcur_t* ae2f_restrict)h_ctx->m_cursors.m_p;

	/** a null cursor must never match.
	 *
	 * clang_equalCursors reports two null cursors as equal, and registers
	 * are created with clang_getNullCursor() where no cursor applies (see
	 * the value-phi temporary in aclexpr/beforevisit/bool1.c). without this
	 * guard a lookup on a cursor that references nothing — an integer
	 * literal index, say — returned whichever such register came first, and
	 * the caller then loaded a value unrelated to the expression. */
	if(clang_Cursor_isNull(c_cur))
		return 0;

	/** walk from the newest register back: an inlined body bound at several
	 * call sites creates one register per carve for the same parameter
	 * cursor, and the body being carved must resolve to the binding created
	 * by THIS carve (the last one), not a stale one an earlier carve made. */
	for(i = h_ctx->m_num_cursor; i; --i) {
		if(
				p[i - 1u].m_class == ACLCUR_REG
				&& clang_equalCursors(p[i - 1u].m_cur, c_cur)
		  ) return p[i - 1u].m_id;
	}

	return 0;
}

#endif

