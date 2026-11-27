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

#ifndef	aclemit_fndecl_sym_h
#define	aclemit_fndecl_sym_h

#include <aclspv.h>
#include <aclctx/err.h>
#include <aclsym/fn.h>

#include <string.h>

/**
 * emit `OpName %id "<declared name>"` for an emitted symbol, when
 * ACLSPV_CTX_FLAG0_SYMBOL asks for it.
 *
 * the name is a debugging aid: calls and linkage resolve by id, and an import
 * already carries its name in the LinkageAttributes decoration. the flag is
 * what keeps the module free of names when they are not wanted.
 *
 * the symbol must have its m_id already assigned.
 * */
static aclspv_compile_t aclemit_fndecl_sym_name(
		const aclsym_fn_t*	c_sym,
		h_aclspv_ctx_t		h_ctx
		) {
	const char*	c_name;
	aclspv_wrd_t	c_len;

	if(!(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL))
		return h_ctx->m_err;

	c_name = aclsym_fn_name(c_sym, h_ctx);

	/** the cache always stores a name; an empty one names nothing. */
	unless(c_name)
		return h_ctx->m_err;

	c_len = (aclspv_wrd_t)strlen(c_name);

	ae2f_ifezerr(aclspv_cfg_symbol(c_name, c_len, c_sym->m_id, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
		return h_ctx->m_err;
	}

	return h_ctx->m_err;
}

#endif
