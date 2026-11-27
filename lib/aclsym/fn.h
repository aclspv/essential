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

/** @file aclsym/fn.h
 *
 * pre-emit function symbols. no aclcur graph, no emission: records only.
 *
 * the caller owns every translation unit's lifetime and must keep it alive
 * until build — aclspv_ctx_compile1 and aclspv_ctx_build1 both document that.
 * the cache leans on it: a symbol records the mark of its declaration for
 * lookup, and ONE cursor, `m_body`, for a body this module still owes. keeping
 * that cursor is what lets a later translation unit's compile finish a body an
 * earlier one had to defer (mutual recursion across sources).
 * */

#ifndef aclsym_fn_h
#define aclsym_fn_h

#include <acldef.h>
#include <aclty/common.h>

/** per-function bits. stored in aclsym_fn_t::m_flag0; an entry-point
 * record (aclsym_entp_t) carries only CONST / PURE, since an entry point
 * is not a call target and therefore has no symbol. */
enum ACLSYM_FN_FLAG0_ {
	/** the declaration has a body in this translation unit. */
	ACLSYM_FN_FLAG0_DEFINED,

	/** the body has been emitted: aclsym_fn_t::m_id is valid. */
	ACLSYM_FN_FLAG0_EMITTED,

	/** no reads of state the function does not own. see aclcur_fn_is_const().
	 *
	 * two sources, one meaning. a body is analysed and the answer replaces
	 * the declared one; a body-less declaration has no body to analyse, so
	 * its __attribute__((const)) stands (aclattr_get_pure). */
	ACLSYM_FN_FLAG0_CONST,

	/** no writes of state the function does not own. see aclcur_fn_is_pure().
	 * same two sources as CONST. */
	ACLSYM_FN_FLAG0_PURE,

	/** the declaration has internal linkage (C `static`, or a C++ function
	 * whose language linkage makes it TU-local). it lives in the separate
	 * static cache and is never imported or exported. */
	ACLSYM_FN_FLAG0_STATIC,

	/** the module already declares this symbol as an import: the build phase
	 * emitted its OpFunction declaration plus `LinkageAttributes Import`.
	 *
	 * an import has m_id as well, but EMITTED says "the body is out" and would
	 * be a lie for it, so the two are kept apart. the bit survives a pass, so a
	 * later source that DEFINES the function does not silently become the
	 * definition of it: that contradiction is what the build reports
	 * (ACLSPV_COMPILE_IMPORT_DEFINED). */
	ACLSYM_FN_FLAG0_IMPORTED,

	/** the declaration is an entry point (carries aclspv_execmodel).
	 *
	 * an entry point is not a call target, so it has no body to call and no id
	 * to export — but it is still a body that has to go out, and the symbol is
	 * the only record of it that outlives the translation unit. the build
	 * phase decides with this flag whether a body is still waiting, without
	 * walking anything. the bit is cleared by a pass whose source does not
	 * declare it. */
	ACLSYM_FN_FLAG0_ENTRY,

	/** the declaration carries an explicit export intent:
	 * `__declspec(dllexport)` or `visibility("default")` (aclattr_get_export).
	 * recorded while the translation unit is alive, so a build that takes no
	 * source can still decorate the export without walking it. */
	ACLSYM_FN_FLAG0_DECL_EXPORT,

	/** the declaration is explicitly `visibility("hidden")`, which overrides
	 * an export-by-default policy. recorded for the same reason. */
	ACLSYM_FN_FLAG0_DECL_HIDDEN,

	/** the module declares this function (an OpFunction with no block) while
	 * its body is still pending: a call reached it before it was emitted, and
	 * mutual recursion would otherwise have every body waiting forever.
	 *
	 * the id is valid from here on, so call sites take the ordinary path. the
	 * definition that follows reuses it — one id, one OpFunction. */
	ACLSYM_FN_FLAG0_DECLARED,

	/** the module already decorates this symbol `LinkageAttributes Export`.
	 *
	 * a pass re-walks the translation unit of an earlier pass too, so the
	 * export pass asks the symbol instead of remembering cursors: an id names
	 * one function, and one function carries one linkage decoration. */
	ACLSYM_FN_FLAG0_EXPORTED,

	ACLSYM_FN_FLAG0_MAX
};

/** the bits that mean the module can name this symbol: its body is out, the
 * build phase declared it as an import, or it was forward-declared because a
 * caller reached it first. anything that wants to CALL a symbol tests this,
 * which is a different question from "is there a body".
 *
 * DECLARED is deliberately NOT here. it only says an id was reserved so a
 * stalled cycle could bind every body before carving any; the body still has
 * to come out, and readiness must keep waiting for it. a call site is the one
 * place that can already go through — see ACLSYM_FN_FLAG0_CALLABLE. */
#define	ACLSYM_FN_FLAG0_NAMED	\
	(((aclspv_wrd_t)1u << (aclspv_wrd_t)ACLSYM_FN_FLAG0_EMITTED)	\
	 | ((aclspv_wrd_t)1u << (aclspv_wrd_t)ACLSYM_FN_FLAG0_IMPORTED))

/** the bits a call site may reference: the module can name it, or has already
 * reserved its id for a body that is on its way. */
#define	ACLSYM_FN_FLAG0_CALLABLE	\
	(ACLSYM_FN_FLAG0_NAMED	\
	 | ((aclspv_wrd_t)1u << (aclspv_wrd_t)ACLSYM_FN_FLAG0_DECLARED))

ae2f_inline_f ae2f_ccconst static aclspv_wrd_t aclsym_fn_flag0(
		enum ACLSYM_FN_FLAG0_ c
		) {
	return ((aclspv_wrd_t)1u) << (aclspv_wrd_t)c;
}


/** one function symbol: the cache pass 1 builds before any body is parsed.
 *
 * the symbol is keyed by the cursor (with a USR fallback for the static cache
 * and a name fallback for the normal cache) and covers declarations and
 * definitions alike. a definition supersedes an earlier prototype.
 *
 * the name is the symbol name: clang_Cursor_getMangling(), or the spelling
 * when clang has no mangling for it. this is what OpName and
 * LinkageAttributes carry, so the display name and the symbol name are one
 * string. C and `extern "C"` declarations come out unmangled; C++
 * declarations carry their Itanium name.
 *
 * the signature is identified by m_type_id alone: the aclty arena interns
 * types, so two declarations of the same declared function type carry one
 * value, and a differing value is a different type.
 * */
typedef struct {
	/* @brief Result ID for function: 0 until the body is emitted. */
	aclspv_id_t	m_id;

	/* @brief aclty id of the declared function type (return type, and the
	 * parameter types in declaration order), interned.
	 *
	 * this id lives in the gather (slave) context; see aclty_subid_t. it is
	 * a comparison key only: equal signatures share one value, a differing
	 * value is a different type. a body or a call builds its own interned
	 * type in the emitting context. */
	aclty_subid_t	m_type_id;

	/* @brief short handle for the declaration: clang_hashCursor.
	 *
	 * the 4-byte mark is the durable key: a later walk of the same
	 * translation unit resolves the declaration again through it, without
	 * holding a cursor. m_mark follows the definition when one supersedes a
	 * prototype. */
	aclspv_wrd_t	m_mark;

	/* @brief the declaration of `m_body`, kept with it: a later pass queues the
	 * pair as one pending item, and the emitter needs both (the declaration for
	 * the signature and the symbol, the body for the carve). */
	CXCursor	m_fn;

	/* @brief the body this module still owes, or a null cursor.
	 *
	 * set when the compile phase has to defer a body — a callee is not
	 * nameable yet — and cleared once it is emitted. it is the ONE cursor the
	 * cache holds, and it is valid because the caller keeps every translation
	 * unit alive until build: a later source's compile picks the deferred body
	 * back up from here and carves it once its callees are out.
	 *
	 * without it, a body deferred in one source could only be finished by
	 * walking that source again — which is exactly what
	 * aclspv_ctx_build4 cannot do.
	 */
	CXCursor	m_body;

	/* @brief the symbol name, as a slice of the cache context's
	 * m_fnlist.m_fn_name. this is the mangled name (or spelling when
	 * clang has no mangling): OpName and LinkageAttributes use it as-is. */
	aclspv_wrd_t	m_idx_name;
	aclspv_wrd_t	m_len_name;

	/* @brief USR slice for a STATIC symbol: the lookup key for the static
	 * cache. two static entities may share one mangled/display name; the
	 * USR keeps them apart. zero for a normal symbol. */
	aclspv_wrd_t	m_idx_usr;
	aclspv_wrd_t	m_len_usr;

	/* @brief enum ACLSYM_FN_FLAG0_ */
	aclspv_wrd_t	m_flag0;
} aclsym_fn_t;

/** the symbol name for c_cur: clang_Cursor_getMangling() with the cursor
 * spelling as the fallback when clang has no mangling. `wr_name` owns the
 * result until disposed. */
const char* aclsym_cursor_name(CXCursor c_cur, CXString* ae2f_restrict wr_name);

/**
 * function symbols: the declaration cache pass 1 builds before any body is
 * handed to the emitter.
 *
 * the cache OUTLIVES a pass. compile and build may each run on a different
 * translation unit, and the module they write is one: a symbol emitted by an
 * earlier pass keeps its id, so a later pass emits the function once and a
 * call in the later source resolves through the same id. only the body-less
 * symbols are re-decided per pass (see aclsym_fn_static_reset_pass).
 *
 * every file-scope FunctionDecl of the MAIN file is recorded, declaration or
 * definition. the cache is what later passes resolve a name or a cursor
 * against, so a call never has to re-walk the AST to learn a callee's
 * signature.
 *
 * the pass runs in h_ctx->m_ctx_slave, a scratch context sharing the user
 * configuration: building a signature means building an aclty type (clang
 * types do not carry our annotation attributes), and that must not push type
 * words into the module being emitted. the cache itself lives there and
 * survives for the whole compile; aclsym_fn_find() looks through the same
 * redirection, so callers keep passing the real context.
 *
 * a normal name has exactly one declared function type: a redeclaration whose
 * aclty type value differs is rejected
 * (ACLSPV_COMPILE_MET_INVAL_FN_REDECL). a redeclaration with the same type
 * merges onto the existing symbol, and a definition supersedes an earlier
 * prototype (its cursor becomes the authoritative one).
 *
 * internal-linkage functions (`static`) are recorded in m_fnlist.m_fn_static
 * instead. they are not importable or exportable, and their lookup key is the
 * cursor USR, not the mangled name: distinct static entities may share a
 * plain C name. their names/USRs live in m_fnlist.m_fn_static_name, a separate
 * arena reset with the static cache when a compile pass starts (or when a
 * build starts without a compiled half).
 *
 * entry points are left out entirely: an entry point is not a call target, so
 * it gets no symbol and its signature is never built. declarations from
 * included headers stay out too: calls into them resolve through clang, and
 * the base header declares overloads this language model does not have.
 * */
aclspv_compile_t aclsym_fn_gather(h_aclspv_ctx_t h_ctx, CXCursor c_root);

/** discard the static cache and its name arena. */
void aclsym_fn_reset_static(h_aclspv_ctx_t h_ctx);

/**
 * discard the symbols the module does not carry, and rebuild the cache from
 * the ones it does.
 *
 * a context outlives one translation unit, but a symbol the module cannot name
 * is a claim about the TU it came from: its 4-byte mark points back into a tree
 * the compiler no longer owns. such a symbol from an EARLIER pass is stale, so
 * it goes — the next pass re-decides the question from its own source.
 *
 * a symbol the module DOES carry is kept: DEFINED or EMITTED (its body is in
 * the emitted words) and IMPORTED (the module declares it). keeping it is what
 * makes a pass idempotent — the second walk finds the function already there
 * and writes no second OpFunction for it, and a call from a later source
 * resolves through the id the earlier pass assigned.
 *
 * the static cache goes back to zero, as it does for a compile: a static
 * symbol is TU-local by definition, so none of it survives a different source.
 *
 * the vectors keep their buffers: only the used lengths move.
 * */
void aclsym_fn_static_reset_pass(h_aclspv_ctx_t h_ctx);

/**
 * drop every deferred body cursor the cache is still holding.
 *
 * the cache carries a body it owes so the next compile can finish it, and that
 * cursor points into the translation unit it came from. build is the end of
 * the line: once it returns the caller may dispose every translation unit, so
 * nothing may still reach into one. this is what makes that safe.
 *
 * a body still owed after build is NOT silently forgotten — it is reported by
 * the build itself (ACLSPV_COMPILE_HOST_PENDING / _RECURSIVE_CALL). this only
 * releases the cursors.
 * */
void aclsym_fn_drop_deferred(h_aclspv_ctx_t h_ctx);

/**
 * the context the symbol caches live in: the scratch sub-context when the host
 * has one, the host otherwise. every aclsym lookup goes through this, so a
 * caller that wants to read the caches directly (rather than by cursor) asks
 * for the same context here.
 * */
x_aclspv_ctx* aclsym_fn_cache(h_aclspv_ctx_t h_ctx);

/**
 * the symbol for c_fn, matched by cursor first and by name as a fallback
 * (clang may hand out a distinct cursor for the same entity). a static
 * declaration is looked up in the static cache by USR.
 *
 * the returned pointer is owned by the slave context's m_fnlist.m_fn or
 * m_fnlist.m_fn_static and is invalidated by the next symbol append: use it
 * before adding another declaration.
 * */
aclsym_fn_t* aclsym_fn_find(CXCursor c_fn, h_aclspv_ctx_t h_ctx);

/** the symbol emitted as c_id (an OpFunction result id), or 0. */
aclsym_fn_t* aclsym_fn_find_id(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

/** the cache a symbol belongs to: the scratch context when there is one. every
 * walk that wants to visit the symbols directly goes through this. */
x_aclspv_ctx* aclsym_fn_cache(h_aclspv_ctx_t h_ctx);

/** the symbol's declared name, NUL-terminated, valid for the compile.
 * returns 0 when the symbol or its storage is invalid. */
const char* aclsym_fn_name(const aclsym_fn_t* c_sym, h_aclspv_ctx_t h_ctx);

/**
 * true when c_sym's body calls a function that has NO body in this
 * translation unit: an importable declaration.
 *
 * the compile phase cannot order such a call — nothing will ever be emitted
 * for the callee here. the symbol of the callee is already made, so only the
 * caller's body is left to the build phase, which makes the import explicit
 * (LinkageAttributes) and lets the declared pure / const attributes describe
 * the call.
 * */
aclspv_bool_t aclsym_fn_needs_import(CXCursor c_fn, h_aclspv_ctx_t h_ctx);

/**
 * true when the BODY c_body calls a cached function that has no body in this
 * translation unit — the compile phase defers such a body, the build phase
 * emits it after declaring the imports.
 *
 * unlike aclsym_fn_needs_import this ignores whether the import already has
 * an id: the answer is a property of the body, not of how far the build has
 * got.
 * */
aclspv_bool_t aclsym_body_needs_import(CXCursor c_body, h_aclspv_ctx_t h_ctx);

/**
 * true when c_body can be emitted now: every call in it targets a symbol the
 * module can already name — one whose id exists (a body emitted by this or an
 * earlier phase, or an import declaration the build phase made).
 *
 * a call to a DEFINED symbol that has not been emitted yet is not ready — the
 * caller must wait for a later pass. a body-less symbol waits for the build
 * phase to declare its import. a self call is never ready: recursion has no
 * SPIR-V form.
 * */
aclspv_bool_t aclsym_body_ready(CXCursor c_body, h_aclspv_ctx_t h_ctx);

/**
 * is the body of c_fn ready to be emitted? the same question as
 * aclsym_body_ready, asked from the function cursor, plus the cases with
 * nothing to emit:
 *  - c_fn has no symbol (an entry point is not a call target), or
 *  - c_fn is a prototype, not a definition in this translation unit.
 * those return true: nothing is waited on.
 *
 * false means: a callee is defined in this translation unit but not out yet,
 * so the caller must be retried after a later body. a cycle never becomes
 * ready — recursion is a hard limitation of the language model, not of this
 * predicate.
 *
 * this is what the emission scheduler (aclemit/bodies.c) orders bodies by.
 * */
aclspv_bool_t aclsym_fn_ready(CXCursor c_fn, h_aclspv_ctx_t h_ctx);

#endif
