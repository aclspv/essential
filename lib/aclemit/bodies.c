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

#include "./bodies.h"
#include <aclctx/err.h>

#include "./fndecl/imports.h"
#include "./fndecl/norm.h"
#include "./fndecl/entp.h"
#include "./root.h"

#include <aclsym/fn.h>

#include <aclcur/common.h>

#include <aclmem.h>
#include <aclutil/wrdemit.h>
#include <aclutil/fn.h>
#include <ae2f/cc/tail.h>

#include <string.h>

/** one main-file body the current phase still has to emit.
 *
 * m_fn is the FunctionDecl (the scheduler asks aclsym_fn_ready about it);
 * m_body is its CompoundStmt (imports and the emitter work on the body).
 * m_entry is set when the declaration has no symbol: an entry point is not a
 * call target, so it is scheduled by body readiness instead. */
struct s_pending {
	CXCursor		m_fn;
	CXCursor		m_body;
	aclspv_bool_t		m_entry;
	aclspv_bool_t		m_done;
};

/**
 * true when a body an earlier compile pass recorded still has no OpFunction
 * in the module.
 *
 * this asks the symbol cache, not the translation unit: a deferred body is
 * exactly a main-file function the gather pass recorded as DEFINED while
 * nothing has emitted it (no EMITTED bit). that covers an entry point too:
 * it is not a call target, but the gather pass records it (ACLSYM_FN_FLAG0_ENTRY)
 * for the same reason a normal symbol is recorded.
 * */
aclspv_bool_t aclemit_body_pending(h_aclspv_ctx_t h_ctx) {
	h_aclspv_ctx_t		cache;
	aclsym_fn_t* ae2f_restrict	base;
	aclspv_wrdcount_t		i;
	aclspv_bool_t			c_static;

	ae2f_ifezerr(h_ctx)		return 0;

	cache = aclsym_fn_cache(h_ctx);

	ae2f_ifezerr(cache)		return 0;

	for(c_static = 0; c_static <= 1; ++c_static) {
		const aclspv_wrdcount_t c_num = c_static
			? cache->m_fnlist.m_num_fn_static
			: cache->m_fnlist.m_num_fn;

		base = (aclsym_fn_t*)(c_static
				? cache->m_fnlist.m_fn_static.m_p
				: cache->m_fnlist.m_fn.m_p);

		unless(base)		continue;

		for(i = 0; i < c_num; ++i) {
			/** a definition this module has not emitted is a body that
			 * still waits: for an import, or for another body. */
			unless(base[i].m_flag0
					& aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED))
				continue;

			if(base[i].m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))
				continue;

			/** an IMPORTED-and-defined symbol is the contradiction the build
			 * reports; it is not a body waiting to be emitted. */
			if(base[i].m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_IMPORTED))
				continue;

			/** nothing calls an entry point, so it has no import to wait
			 * for: it waits only for the bodies it calls. */
			if(base[i].m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY)) {
				if(aclemit_fndecl_entp_exists_by_mark(base[i].m_mark, h_ctx))
					continue;

				return 1;
			}

			return 1;
		}
	}

	return 0;
}

/**
 * true when a body still pending can no longer be completed by ANY phase —
 * its waiting is a call cycle, not a missing import.
 *
 * an internal-linkage function is the whole answer. it can never be imported
 * (aclspv imports only external-linkage functions), so its body must come out
 * of the translation unit that declares it. the compile phase already sorts
 * the bodies of one translation unit by dependency, so a static body that is
 * still pending after it means the dependency graph has a cycle.
 * */
aclspv_bool_t aclemit_body_cyclic(h_aclspv_ctx_t h_ctx) {
	h_aclspv_ctx_t		cache;
	aclsym_fn_t* ae2f_restrict	base;
	aclspv_wrdcount_t		i;

	ae2f_ifezerr(h_ctx)		return 0;

	cache = aclsym_fn_cache(h_ctx);

	ae2f_ifezerr(cache)		return 0;

	base = (aclsym_fn_t*)cache->m_fnlist.m_fn_static.m_p;

	unless(base)		return 0;

	for(i = 0; i < cache->m_fnlist.m_num_fn_static; ++i) {
		unless(base[i].m_flag0
				& aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED))
			continue;

		if(base[i].m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))
			continue;

		return 1;
	}

	return 0;
}

struct s_collect {
	x_aclspv_vec*		m_buf;
	h_aclspv_ctx_t		m_ctx;
	aclspv_wrd_t		m_num;
};

static enum CXChildVisitResult s_collect_visit(
		CXCursor		c_cur,
		CXCursor		c_par,
		CXClientData		rdwr_client
		) {
	struct s_collect* ae2f_restrict p = rdwr_client;
	aclsym_fn_t* ae2f_restrict	p_sym;
	struct s_pending		ITEM;
	aclspv_wrd_t			c_n;

	if(c_cur.kind != CXCursor_CompoundStmt
			|| !(c_par.kind == CXCursor_FunctionDecl
				|| aclutil_fn_is_method(c_par)))
		return CXChildVisit_Recurse;

	if(aclutil_fn_is_std_move_decl(c_par))
		return CXChildVisit_Continue;

	unless(clang_Location_isFromMainFile(clang_getCursorLocation(c_par)))
		return CXChildVisit_Continue;

	p_sym = aclsym_fn_find(c_par, p->m_ctx);

	if(p_sym && (p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY))) {
		/** an entry point the gather pass recorded: it has a symbol, but not
		 * as a call target — nothing calls it, nothing imports or exports it.
		 * one this module already carries is done — a later pass re-walks the
		 * translation unit the earlier one saw, and a second OpEntryPoint for
		 * one function is not a module. */
		if(aclemit_fndecl_entp_exists_by_mark(p_sym->m_mark, p->m_ctx))
			return CXChildVisit_Continue;

		ITEM.m_entry = 1;
	}
	else if(p_sym) {
		/** a body emitted by an earlier pass has nothing left to emit. the
		 * IMPORTED case is not so simple: a later source may DEFINE it, and
		 * that contradiction is the build's to report, so such a body is
		 * still collected (s_settled decides). */
		if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))
			return CXChildVisit_Continue;

		ITEM.m_entry = 0;
	}
	else {
		/** no symbol at all: an entry point the gather pass could not record
		 * (it was not from the main file, or it is an entry the cache dropped).
		 * the same rule applies. */
		if(aclemit_fndecl_entp_exists(c_par, p->m_ctx))
			return CXChildVisit_Continue;

		ITEM.m_entry = 1;
	}

	ITEM.m_fn	= c_par;
	ITEM.m_body	= c_cur;
	ITEM.m_done	= 0;

	c_n = p->m_num;

	ae2f_ifnezerr(s_aclmem_cgrow2(
				p->m_buf
				, c_n + 1
				, sizeof(struct s_pending)
				, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
		return CXChildVisit_Break;

	((struct s_pending*)p->m_buf->m_p)[c_n] = ITEM;
	p->m_num = c_n + 1;

	/** the body is recorded whole: do not descend into it. */
	return CXChildVisit_Continue;
}

/** one emission: bootstrap the arena and hand the body to its emitter. */
static aclspv_compile_t s_emit(struct s_pending* c_it, h_aclspv_ctx_t h_ctx) {
	h_ctx->m_num_cursor	= 0;

	ae2f_ifezerr(aclcur_cfg_root(ACLSPV_ID_VOID, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
		return h_ctx->m_err;
	}

	h_ctx->m_work_data	= h_ctx;
	h_ctx->m_work_nxt	= aclemit_root;

	if(c_it->m_entry)
		(void)aclemit_fndecl_entp(c_it->m_body, c_it->m_fn, h_ctx);
	else
		(void)aclemit_fndecl_norm(c_it->m_body, c_it->m_fn, h_ctx);

	return h_ctx->m_err;
}

/** can this body be emitted now? every callee must already be nameable. */
static aclspv_bool_t s_ready(
		const struct s_pending*	c_it,
		h_aclspv_ctx_t		h_ctx
		) {
	/** a call the inliner carves needs no callee id, so such a callee is
	 * not worth waiting for: its body is carved right at the call site.
	 * that is what lets an always_inline callee be defined after (or
	 * inside) its caller. */
	if(c_it->m_entry
			? aclsym_body_ready(c_it->m_body, h_ctx)
			: aclsym_fn_ready(c_it->m_fn, h_ctx))
		return 1;

	return aclutil_body_calls_ready(c_it->m_fn, h_ctx);
}

/** the symbol this item emits a body for, or 0 for an entry point. */
static aclsym_fn_t* s_sym(
		const struct s_pending*	c_it,
		h_aclspv_ctx_t		h_ctx
		) {
	return c_it->m_entry ? 0 : aclsym_fn_find(c_it->m_fn, h_ctx);
}

/**
 * settle one item against what the module already carries, before anything is
 * emitted for it:
 *  - done: an earlier pass emitted this body.
 *  - done, as an error: the module declares this function as an import — the
 *    pass that saw a call to it found no body, and this one found one.
 *
 * true means the item needs no queue slot.
 * */
static aclspv_bool_t s_settled(
		struct s_pending*			c_it,
		h_aclspv_ctx_t			h_ctx,
		aclspv_bool_t* ae2f_restrict	wr_done
		) {
	aclsym_fn_t* ae2f_restrict p_sym;

	*wr_done = 0;

	p_sym = s_sym(c_it, h_ctx);

	unless(p_sym)
		return 0;

	if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_IMPORTED)) {
		/** SPIR-V gives one answer per function: a declaration that promises
		 * `Import` and a definition in this module cannot both stand. */
		err_puts("a function this module imports is defined by a later pass");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_IMPORT_DEFINED);
		*wr_done = 1;
		return 1;
	}

	if(p_sym->m_flag0 & ACLSYM_FN_FLAG0_NAMED) {
		*wr_done = 1;
		return 1;
	}

	return 0;
}

/** declare the imports every pending body needs, and prepend the declaration
 * words before the definitions already in the module.
 *
 * SPIR-V requires every function declaration to precede every definition, and
 * the compile phase may already have written definitions, so the words are
 * collected aside and moved in front. the decorations go straight to the
 * decorate section, where order does not matter. */
static aclspv_compile_t s_imports(
		h_aclspv_ctx_t		h_ctx,
		struct s_pending*	items,
		aclspv_wrd_t		c_num
		) {
	x_aclspv_vec		decls;
	aclspv_sz_t		c_decl = 0;
	aclspv_wrd_t		i;
	aclspv_wrd_t* ae2f_restrict base;

	aclmem_init(&decls, h_ctx);

	for(i = 0; i < c_num; ++i) {
		ae2f_ifnezerr(aclemit_fndecl_imports(
					items[i].m_body
					, &decls
					, &c_decl
					, h_ctx)) {
			aclmem_fini(&decls);
			return h_ctx->m_err;
		}
	}

	if(c_decl) {
		const aclspv_sz_t c_old = h_ctx->m_count.m_fndef;

		ae2f_ifnezerr(s_aclmem_cgrow2(
					&h_ctx->m_section.m_fndef
					, c_old + c_decl
					, sizeof(aclspv_wrd_t)
					, ACLSPV_COMPILE_ALLOC_FAILED)) {
			aclmem_fini(&decls);
			return h_ctx->m_err;
		}

		base = aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fndef);

		memmove(base + c_decl, base, (size_t)aclutil_count_to_sz(c_old));
		memcpy(base, decls.m_p, (size_t)aclutil_count_to_sz(c_decl));

		h_ctx->m_count.m_fndef = c_old + c_decl;
	}

	aclmem_fini(&decls);

	return h_ctx->m_err;
}

/** collect, declare imports when building, then emit in dependency order. */
ae2f_inline_f static aclspv_compile_t s_run(
		h_aclspv_ctx_t	h_ctx,
		CXCursor	c_root,
		aclspv_bool_t	c_build
		) {
	x_aclspv_vec		buf;
	struct s_collect	col;
	struct s_pending*	items;
	aclspv_wrd_t		c_num;
	aclspv_wrd_t		i;
	aclspv_bool_t		c_progress;

	aclmem_init(&buf, h_ctx);

	col.m_buf	= &buf;
	col.m_ctx	= h_ctx;
	col.m_num	= 0;

	clang_visitChildren(c_root, s_collect_visit, &col);

	aclctx_err_check(h_ctx) {
		aclmem_fini(&buf);
		return h_ctx->m_err;
	}

	items = buf.m_p;
	c_num = col.m_num;



	/** the imports must exist before readiness can accept the callers. */
	if(c_build) {
		ae2f_ifnezerr(s_imports(h_ctx, items, c_num)) {
			aclmem_fini(&buf);
			return h_ctx->m_err;
		}
	}

	/** dependency queue: emit what is nameable, repeat until no progress. */
	do {
		c_progress = 0;

		for(i = 0; i < c_num; ++i) {
			unless(items[i].m_done) {
				aclspv_bool_t c_done;

				/** a body an earlier pass emitted, or an import this module
				 * already declares: neither has anything to emit. */
				if(s_settled(&items[i], h_ctx, &c_done)) {
					if(c_done) {
						items[i].m_done = 1;
						c_progress = 1;
					}

					aclctx_err_check(h_ctx) {
						aclmem_fini(&buf);
						return h_ctx->m_err;
					}

					continue;
				}

				unless(s_ready(&items[i], h_ctx))
					continue;

				ae2f_ifnezerr(s_emit(&items[i], h_ctx)) {
					aclmem_fini(&buf);
					return h_ctx->m_err;
				}

				items[i].m_done = 1;
				c_progress = 1;
			}
		}
	} while(c_progress);

	if(c_build) {
		for(i = 0; i < c_num; ++i) {
			unless(items[i].m_done) {
				/** a call cycle (recursion) or a callee that never got
				 * a body: nothing in the target can express it. */
				_aclctx_err(h_ctx, ACLSPV_COMPILE_RECURSIVE_CALL);
				break;
			}
		}
	}

	aclmem_fini(&buf);

	return h_ctx->m_err;
}

aclspv_compile_t aclemit_compile_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	return s_run(h_ctx, c_root, 0);
}

aclspv_compile_t aclemit_build_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	return s_run(h_ctx, c_root, 1);
}
