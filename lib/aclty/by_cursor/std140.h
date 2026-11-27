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

#ifndef	aclty_by_cursor_std140_h
#define aclty_by_cursor_std140_h

#include "../by_cursor.h"

static aclspv_id_t aclty_mk_by_cursor_std140(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx) {
	aclty_mk_by_cursor_info_t a;
	a.m_add_struct_mbr	= aclty_add_struct_mbr_std140;
	a.m_array_stride	= 16;
	return aclty_mk_by_cursor(
			c_cursor
			, c_type
			, h_ctx
			, &a);
}

#endif
