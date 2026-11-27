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

aclspv_compile_t EMIT_aclcur_jmp(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
#if	__ACLSPV_CHECK_EXTRA
	ae2f_ifezerr(h)				return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifezerr(h->m_class == ACLCUR_J)	return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifezerr(h_ctx)			return ACLSPV_COMPILE_UNHANDLED;
#else
	ae2f_assume(h);
	ae2f_assume_strict(h->m_class == ACLCUR_J);
	ae2f_assume(h_ctx);
#endif

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_2(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, SpvOpBranch
				, h->m_d.m_j.m_lbl
				))
		return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

#if	__ACLSPV_CHECK_EXTRA
	{
		ae2f_ifezerr(aclcur_get_node(h->m_d.m_j.m_lbl, h_ctx))
			return ACLSPV_COMPILE_UNHANDLED;
	}
#endif

	return ACLSPV_COMPILE_OK;
}
