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
 *
 * this is exactly aclspv_ctx_compile followed by aclspv_ctx_build4; use
 * aclspv_ctx_build4 when the source is already compiled and must not be
 * walked again.
 *
 * @see	aclspv_ctx_build4
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

/**
 * @brief	BUILD phase from what the context already carries; no source is
 *		taken.
 *
 * closes the module from what earlier aclspv_ctx_compile passes recorded. no
 * part of it walks a translation unit:
 *
 *  - the export intent of a declaration, recorded by the gather pass while the
 *    translation unit was alive (ACLSYM_FN_FLAG0_DECL_EXPORT / _DECL_HIDDEN),
 *    is what decorates the exports of every emitted function.
 *
 *  - the Linkage capability goes in when the module has no entry point.
 *
 *  - an import is declared from the symbol too: the declared function type it
 *    carries (m_type_id) is already interned in this context, so a called
 *    function with no body needs no cursor to be declared.
 *
 * only a body the compile phase DEFERRED needs the source: it must be carved
 * again, and that is a walk into its translation unit. build4 was given none,
 * so it reports that as ACLSPV_COMPILE_HOST_PENDING instead.
 *
 * the compile phase must have run at least once. otherwise the module would
 * silently close without any source vouching for it, reported as
 * ACLSPV_COMPILE_UNHANDLED.
 *
 * a deferred body can never settle without its translation unit, so a caller
 * that disposes one must leave nothing pending: a body only defers when it
 * calls something not yet nameable (an import, or a body defined later in the
 * same translation unit), and compiling one source per translation unit
 * leaves no such body behind. build4 is then the closing step of a compile |
 * build split — compile each source as it arrives, dispose it, and close the
 * module once at the end.
 *
 * if a translation unit did defer a body, close the module while that
 * translation unit is alive: aclspv_ctx_build1 still declares the import and
 * emits it. build4 takes no source, so it reports the deferral as
 * ACLSPV_COMPILE_HOST_PENDING rather than walking a translation unit that may
 * already be gone.
 *
 * @see	aclspv_ctx_build
 * @see	aclspv_ctx_compile
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_build4(
		const h_aclspv_ctx_t	h_ctx
		);

#endif
