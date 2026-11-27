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

#ifndef	aclutil_cxtp_struct_h
#define aclutil_cxtp_struct_h

#include "./common.h"

enum ACLTY_STRUCT_FIELD0 {
	ACLTY_STRUCT_FIELD0_BLOCK = 1,
	/** the record derives from a C union: its C90 initializer is a
	 * single element for the first member; the rest are null. */
	ACLTY_STRUCT_FIELD0_UNION = 2
};

/** @see enum ACLTY_STRUCT_FIELD0 */
typedef aclspv_wrd_t aclty_struct_field0_t;

aclspv_id_t aclty_mk_struct_single_blk(
		const aclspv_id_t	c_type_member,
		const aclty_struct_field0_t	
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		);

/** STRUCT DECORATION */

aclspv_id_t aclty_cfg_struct(h_aclspv_ctx_t h_ctx);
aclspv_id_t aclty_cfg_struct2(h_aclspv_ctx_t h_ctx, aclspv_id_t c);
aclty_t* aclty_build_struct(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

#include "./struct/mbr.h"
#include "./struct/dcr.h"

#endif
