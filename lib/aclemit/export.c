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

#include "./export.h"
#include <aclctx/err.h>

#include <aclsym/fn.h>

#include <aclutil/capability.h>
#include <aclutil/linkage.h>

#include <ae2f/pat/dbg.h>

/** decorate every symbol of `h_ctx`'s symbol caches that the module can name.
 *
 * the symbols live in the gather's scratch context (or directly in `h_ctx`
 * when it has none), while the decoration is a word of the MODULE: the section
 * and the user configuration are `h_ctx`'s either way. reading the intent from
 * the symbol instead of from a cursor is what lets a build phase that takes no
 * source still close the module.
 * */
static aclspv_compile_t s_cache(h_aclspv_ctx_t h_ctx) {
	aclsym_fn_t* ae2f_restrict	base;
	aclspv_wrdcount_t		i;
	aclspv_bool_t			c_static;
	x_aclspv_ctx* ae2f_restrict	h_cache;

	h_cache = h_ctx->m_ctx_slave ? h_ctx->m_ctx_slave : h_ctx;

	for(c_static = 0; c_static <= 1; ++c_static) {
		const aclspv_wrdcount_t c_num = c_static
			? h_cache->m_fnlist.m_num_fn_static
			: h_cache->m_fnlist.m_num_fn;

		base = (aclsym_fn_t*)(c_static
				? h_cache->m_fnlist.m_fn_static.m_p
				: h_cache->m_fnlist.m_fn.m_p);

		unless(base)		continue;

		for(i = 0; i < c_num; ++i) {
			aclsym_fn_t* ae2f_restrict	p_sym = &base[i];
			aclspv_bool_t			c_export;

			/** an entry point is not callable: it is reached through
			 * OpEntryPoint, never through a linkage decoration. */
			if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY))
				continue;

			/** an internal-linkage definition has no ABI name to export:
			 * it lives in the static cache and stays inside this module. */
			if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_STATIC))
				continue;

			/** only an emitted definition has an OpFunction to decorate. */
			unless(p_sym->m_flag0 & (aclspv_wrd_t)(
						aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)
						| ACLSYM_FN_FLAG0_NAMED))
				continue;

			c_export = (aclspv_bool_t)(
					(p_sym->m_flag0
					 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DECL_EXPORT))
					|| (!(p_sym->m_flag0
							& aclsym_fn_flag0(ACLSYM_FN_FLAG0_DECL_HIDDEN))
						&& (h_ctx->m_usrcfg.m_flag0
							& ACLSPV_CTX_FLAG0_EXPORT_BY_DEFAULT)));

			unless(c_export)
				continue;

			/** one id, one linkage decoration: a pass that re-walks this
			 * translation unit has nothing left to say about it. */
			if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EXPORTED))
				continue;

			ae2f_ifezerr(aclutil_add_capability_by_flag(
						h_ctx, SpvCapabilityLinkage))
				return h_ctx->m_err;

			{
				aclspv_sz_t c;

				ae2f_ifezerr(c = aclutil_emit_linkage(
							&h_ctx->m_section.m_decorate
							, h_ctx->m_count.m_decorate
							, p_sym->m_id
							, aclsym_fn_name(p_sym, h_cache)
							, (aclspv_wrd_t)SpvLinkageTypeExport)) {
					_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
					return h_ctx->m_err;
				}

				h_ctx->m_count.m_decorate = c;
			}

			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_EXPORTED);
		}
	}

	return h_ctx->m_err;
}

aclspv_compile_t aclemit_export(h_aclspv_ctx_t h_ctx) {
	aclspv_compile_t ret;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	/** the export intent was recorded by the gather pass, while the
	 * translation unit was alive: decorating here needs no cursor, which is
	 * what lets a build phase that takes no source still close the module. */
	ret = s_cache(h_ctx);

	return ret;
}
