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

#include "./nil.h"
#include <aclval.h>
#include <aclutil/emitx.h>
#include <aclctx/id.h>

#include <aclty/default.h>

aclspv_id_t aclval_cfg_nil(aclty_id_t c_ty, h_aclspv_ctx_t h) {
	aclval_t* ae2f_restrict v = aclval_cfg_node2(0, c_ty, h);
	ae2f_ifezerr(v) return 0;
	aclty_mk_default(c_ty, h);
	if(v->m_nil)	return v->m_nil;
	else ae2f_ifezerr(h->m_count.m_types = aclutil_emitx_3(
				&h->m_section.m_types
				, h->m_count.m_types
				, SpvOpConstantNull
				, c_ty
				, v->m_nil = aclctx_newid(h)
				)) return 0;

	return v->m_nil;
}
