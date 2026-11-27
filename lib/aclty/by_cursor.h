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

#ifndef aclutil_cxtp_by_cursor_h
#define aclutil_cxtp_by_cursor_h

#include "./common.h"
#include "./layout.h"

typedef aclspv_id_t aclty_mk_by_cursor_t(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx);

typedef aclty_layout_interface_t aclty_mk_by_cursor_info_t;

/* Returns the type id of type */
aclty_id_t aclty_mk_by_cursor(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx,
		const aclty_mk_by_cursor_info_t* ae2f_restrict	rd_info
		);

/** the one entry for building a type from a cursor: it applies the declared
 * layout (std430 unless aclspv_layout says otherwise) and parses whatever
 * compound type the type needs. "default" names that default-layout path,
 * not a degraded variant.
 *
 * the cursor carries the annotations (aclspv_storage_class, aclspv_layout,
 * ...), which the clang type does not; pass the DECLARED type, not a
 * canonicalised one, or the typedef holding them is already gone. */
aclty_id_t aclty_mk_by_cursor_default(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		); 

ae2f_inline_f static aclty_id_t aclty_mk_by_cursor_default2(
		const CXCursor		c_cur,
		const h_aclspv_ctx_t	h_ctx
		) {
	return aclty_mk_by_cursor_default(c_cur, clang_getCursorType(c_cur), h_ctx);
}

#endif
