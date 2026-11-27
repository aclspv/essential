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

#ifndef	aclemit_fndecl_imports_h
#define	aclemit_fndecl_imports_h

#include <aclspv.h>
#include <aclsym/fn.h>

/**
 * declare the functions c_body calls that have no body in this translation
 * unit.
 *
 * a body-less normal function is an import: it is referenced by id and
 * resolved at link time, so the module carries an OpFunction declaration
 * (no block) plus
 *
 *   OpDecorate %id LinkageAttributes "<name>" Import
 *
 * and the Linkage capability.
 *
 * the declaration words go to h_wrds / *wr_count, NOT straight into the
 * module: SPIR-V requires every function declaration to precede every
 * function definition, and definitions may already be emitted by the build
 * phase's start. the caller prepends them. the decorations go straight to
 * the decorate section (order there does not matter).
 * */
aclspv_compile_t aclemit_fndecl_imports(
		CXCursor		c_body,
		x_aclspv_vec*		h_wrds,
		aclspv_wrdcount_t* ae2f_restrict	wr_count,
		h_aclspv_ctx_t		h_ctx
		);

/**
 * forward-declare one symbol whose body is in the queue but not out yet.
 *
 * a call that reaches a pending body first needs nothing but its id, and
 * emitting the declaration here is what lets two bodies that call each other
 * both be carved: neither has to wait for the other to finish. the words are
 * collected like an import's (`h_wrds` / *wr_count, the caller prepends them
 * before every definition), but no linkage decoration is written — the
 * function is defined in this module, not imported.
 *
 * a symbol that already has an id is left alone, so calling this for the whole
 * queue costs one pass over bodies already named.
 * */
/**
 * give p_sym an id, writing nothing to the module.
 *
 * a call site needs the callee's id, not its declaration: SPIR-V resolves an
 * OpFunctionCall by id and the definition may come later. reserving the id is
 * therefore all a stalled queue needs — every body of a cycle can be bound
 * before any of them is carved, and no placeholder OpFunction is left behind.
 * */
aclspv_compile_t aclemit_fndecl_reserve(
		aclsym_fn_t*		p_sym,
		h_aclspv_ctx_t		h_ctx
		);

#endif
