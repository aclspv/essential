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

#include "./emit.h"
#include "../common.h"
#include <aclutil/emitx.h>

aclspv_compile_t EMIT_aclcur_ret(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {

#if	__ACLSPV_CHECK_EXTRA
#define	s_assume(c, c_err)	ae2f_ifezerr((c)) return (c_err)
#else
#define	s_assume(c, c_err)	ae2f_assume((c))
#endif

	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	if(h->m_val) {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpReturnValue
					, h->m_val
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	} else {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = emit_opcode(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpReturn, 0
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	}

	return ACLSPV_COMPILE_OK;
}
