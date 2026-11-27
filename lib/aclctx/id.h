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

#ifndef aclctx_id_h
#define aclctx_id_h

#include <acldef.h>

#if	__ACLSPV_ATOMIC_ID
#include <c89atomic.h>
#endif


ae2f_inline_f static aclspv_id_reserved_t aclctx_newid(h_aclspv_ctx_t h) {
#if	__ACLSPV_ATOMIC_ID
	aclspv_id_t new_ = c89atomic_fetch_add_32(&h->m_id, 1u);
#else
	aclspv_id_t new_ = h->m_id++;
#endif

#if	1
	ae2f_ifnezerr_strict(new_ <= new_ - 1u)
		return 0;
#endif

	return new_;
}

#endif
