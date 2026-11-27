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

#ifndef	aclconst_align_h
#define aclconst_align_h

#include "./specsize.h"
#include <acldef.h>

aclconst_specsz	aclconst_align_from_size(
		aclconst_specsz	c,
		h_aclspv_ctx_t	h_ctx
		);

aclconst_specsz	aclconst_alignup(
		aclconst_specsz	c,
		aclconst_specsz	c_algn,
		h_aclspv_ctx_t	h_ctx
		);

ae2f_inline_f static aclconst_specsz aclconst_alignup1(
		aclconst_specsz	c,
		aclspv_wrd_t	c_algn,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz algn;
	algn.m_id = 0;
	algn.m_lit.m_wrd = c_algn;

	return aclconst_alignup(c, algn, h_ctx);
}

aclconst_specsz	aclconst_align_from_size2(
		aclspv_wrd_t	c,
		h_aclspv_ctx_t	h_ctx
		);

#endif
