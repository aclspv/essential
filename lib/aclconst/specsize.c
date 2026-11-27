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


#include "./specsize.h"
#include <aclctx/err.h>
#include <aclval.h>
#include <aclty/sclr.h>

aclspv_bool_t aclconst_is_specsz_same(aclconst_specsz c_x, aclconst_specsz c_y, h_aclspv_ctx_t h_ctx) {
	if(c_x.m_id && c_y.m_id) {
		return c_x.m_id == c_y.m_id;
	}

	if(!c_x.m_id && !c_y.m_id) {
		return c_x.m_lit.m_max == c_y.m_lit.m_max;
	}

	{
		aclspv_id_t	szty = aclty_get_sclr_uint(h_ctx->m_usrcfg.m_byte_size_type << 3);
		ae2f_ifezerr(aclty_mk_default(szty, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_ID);
			return 0;
		}

		unless(c_x.m_id) c_x.m_id = aclval_cfg_by_enum(
				c_x.m_lit.m_max, h_ctx, (enum ACLSPV_ID_)szty);

		unless(c_y.m_id) c_y.m_id = aclval_cfg_by_enum(
				c_y.m_lit.m_max, h_ctx, (enum ACLSPV_ID_)szty);

		ae2f_ifnezerr_strict(!c_x.m_id && !c_y.m_id) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_ID);
			return 0;
		}

		return c_x.m_id == c_y.m_id;
	}
}
