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

#ifndef	aclspv_ctx_comp_h
#define aclspv_ctx_comp_h

#include "./common.h"
#include <aclspv/wrd.h>

#include <clang-c/Index.h>

#include <aclspv/enum/err.h>
#include <aclspv/link.h>
#include <ae2f/Cxx/extern.h>

/**
 * @brief	COMPILE phase: gather the function symbols of `c_cur_root` and
 *		emit the bodies that need no import.
 *
 * bodies are emitted in dependency order (a callee before its callers), so
 * the definition order in the source does not matter. a body whose callees
 * are not all nameable yet is skipped here (its symbol is still made): a call
 * to a function with no body in this translation unit, or to one the
 * scheduler could not reach. the build phase completes those.
 * the translation unit must stay alive until the build call: the cursors
 * held by this phase are the compiler's handles into it, and the context
 * never takes ownership of it.
 *
 * both phases may run any number of times, on as many translation units as the
 * caller likes: the context accumulates one module, and a function emitted by
 * an earlier pass keeps the id it was given rather than being emitted twice.
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile(
		const h_aclspv_ctx_t	h_ctx,
		CXCursor		c_cur_root
		);

/**
 * @brief	COMPILE phase from a caller-owned translation unit.
 *
 * the TU is NOT disposed and MUST outlive the matching aclspv_ctx_build1.
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile1(
		const h_aclspv_ctx_t	h_ctx,
		CXTranslationUnit	h_tu
		);

/**
 * @brief	BUILD phase: complete what the compile phase deferred — declare
 *		the imports (LinkageAttributes Import), emit the bodies that
 *		call them, and decorate the exports.
 *
 * runs the compile phase first when it has not run. the TU must still be
 * alive: the build phase re-walks it.
 *
 * a function this module already declares as an import has no body to gain:
 * a later pass that finds a definition of it reports
 * ACLSPV_COMPILE_IMPORT_DEFINED.
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build(
		const h_aclspv_ctx_t	h_ctx,
		CXCursor		c_cur_root
		);

/** BUILD phase from a caller-owned translation unit (not disposed). */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build1(
		const h_aclspv_ctx_t	h_ctx,
		CXTranslationUnit	h_tu
		);

/** parse + build; the TU is owned and disposed here. */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build2(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt,
		CXIndex							h_cxidx
		);

/** create the index + parse + build; everything is owned here. */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build3(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
		);

#endif
