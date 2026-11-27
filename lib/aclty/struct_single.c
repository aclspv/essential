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

#include "../aclty.h"
#include "./struct.h"
#include "./struct/mbr.h"
#include "./struct/dcr.h"

aclspv_id_t aclty_mk_struct_single_blk(
		const aclspv_id_t	c_type_member,
		const aclty_struct_field0_t
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t id = aclty_cfg_struct(h_ctx);
	aclty_t* ae2f_restrict cxtp = aclty_find(id, h_ctx);
	ae2f_ifezerr(cxtp) {
		return 0;
	}

	if(c_memdecorate_offset ^ 0xFFFFFFFF) {
		aclty_t* ae2f_restrict mbr = aclty_add_struct_mbr_un(cxtp, c_type_member, h_ctx);
		ae2f_ifezerr(mbr)	return 0;
		cxtp = aclty_find(id, h_ctx);
	}

	cxtp = aclty_find(id, h_ctx);

	if(c_decorate_field0 & ACLTY_STRUCT_FIELD0_BLOCK) {
		(void)aclty_cfg_struct_dcr(cxtp, SpvDecorationBlock, h_ctx);
	}

	cxtp = aclty_find(id, h_ctx);

	{
		aclty_t* ae2f_restrict ret = aclty_build_struct(id, h_ctx);
		ae2f_ifezerr(ret) return 0;
		return ret->m_type_id;
	}
}
