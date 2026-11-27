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

#include <aclspv.h>
#include <aclctx/err.h>
#include <ae2f/cc.h>
#include <aclutil/emitx.h>

ACLSPV_ABI_LINK ae2f_ccpure aclspv_bool_t aclspv_chck_capability_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const enum SpvCapability_		c_cap
		) {
	const aclspv_wrd_t	COUNT = rd_ctx 
		? rd_ctx->m_count.m_capability >> (aclspv_wrd_t)1 
		: (aclspv_wrd_t)0;

#define	KEY	((aclspv_wrd_t)c_cap)

	aclspv_wrd_t* const ae2f_restrict BASE	= rd_ctx->m_section.m_capability.m_p;

	aclspv_wrd_t	LR[2];

	ae2f_ifezerr(rd_ctx)		return 0;
	ae2f_ifezerr(c_cap)		return 0;
	ae2f_ifnezerr(rd_ctx->m_err)	return 0;
	ae2f_ifezerr(BASE)		return 0;

#define	LEFT		LR[1]
#define	RIGHT		LR[0]


	LEFT	= 0;
	RIGHT	= COUNT;

	while(LEFT < RIGHT) {
		const aclspv_wrd_t	MID = LEFT + ((RIGHT - LEFT) >> 1);
		if(BASE[(MID << 1) + 1] == KEY)	return	1;
		LR[BASE[(MID << 1) + 1] < KEY] = MID + (BASE[(MID << 1) + 1] < KEY);
	}

	return LEFT < COUNT && BASE[(LEFT << 1) + 1] == KEY;

#undef	LEFT
#undef	RIGHT
#undef	KEY
}

ACLSPV_ABI_LINK aclspv_bool_t aclspv_add_capability_from_ctx(
		h_aclspv_ctx_t				h_ctx,
		const enum SpvCapability_		c_cap
		) {
	const aclspv_wrd_t	COUNT = h_ctx 
		? (h_ctx->m_count.m_capability >> (aclspv_wrd_t)1)
		: (aclspv_wrd_t)0;

#define	KEY	((aclspv_wrd_t)c_cap)

	aclspv_wrd_t* ae2f_restrict BASE	= h_ctx->m_section.m_capability.m_p;

	aclspv_wrd_t	LR[2];

	ae2f_ifezerr(h_ctx)		return 0;
	aclctx_err_check(h_ctx) return 0;
	ae2f_ifezerr(c_cap)		return 0;

#define	LEFT		LR[1]
#define	RIGHT		LR[0]

	LEFT	= 0;
	RIGHT	= COUNT;

	ae2f_ifneznrm(BASE) while(LEFT < RIGHT) {
		const aclspv_wrd_t	MID = LEFT + ((RIGHT - LEFT) >> 1);
		if(BASE[(MID << 1) + 1] == KEY)	return	1;
		LR[BASE[(MID << 1) + 1] < KEY] = MID + (BASE[(MID << 1) + 1] < KEY);
	}

	if(LEFT < COUNT && BASE[(LEFT << 1) + 1] == KEY) return 1;

	ae2f_ifezerr(h_ctx->m_count.m_capability = aclutil_emitx_2(
				&h_ctx->m_section.m_capability
				, h_ctx->m_count.m_capability
				, SpvOpCapability
				, KEY)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_CAPABILITY);
		return 0;
	}

	BASE	= ((aclspv_wrd_t*)(h_ctx->m_section.m_capability.m_p));

	if(COUNT > LEFT) {
		aclspv_wrd_t		C = COUNT;

		while(C-- > LEFT) {
			BASE[(C << 1) + 3] = BASE[(C << 1) + 1];
		}
	}	/** memmove */


	BASE[(LEFT << 1) + 1] = KEY;
	return 1;

#undef	LEFT
#undef	RIGHT
#undef	KEY
}
