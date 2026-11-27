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

#endif
