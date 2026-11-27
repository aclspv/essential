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
#include "./sync.h"

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

/** append one entry to the queue `p` collects from the current walk. */
static int s_push(
		struct s_collect* ae2f_restrict	p,
		CXCursor		c_fn,
		CXCursor		c_body,
		aclspv_bool_t		c_entry
		) {
	struct s_pending	ITEM;
	const aclspv_wrd_t	c_n = p->m_num;

	ITEM.m_fn	= c_fn;
	ITEM.m_body	= c_body;
	ITEM.m_entry	= c_entry;
	ITEM.m_done	= 0;

	ae2f_ifnezerr(s_aclmem_cgrow2(
				p->m_buf
				, c_n + 1
				, sizeof(struct s_pending)
				, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
		return 1;

	((struct s_pending*)p->m_buf->m_p)[c_n] = ITEM;
	p->m_num = c_n + 1;

	return 0;
}

/**
 * queue the bodies an EARLIER translation unit had to defer.
 *
 * a body defers when a callee is not nameable yet, and only another source can
 * change that: `f` in a.c waits for `g`, and b.c defines it. the symbol kept
 * that body's cursors for exactly this (`m_body`), so b.c's compile can finish
 * a.c's body — without re-walking a.c, and without leaving it for the build
 * phase.
 *
 * the caller's contract is what makes the cursors valid: every translation
 * unit stays alive until build.
 * */
static aclspv_compile_t s_collect_deferred(
		struct s_collect* ae2f_restrict	p
		) {
	x_aclspv_ctx* ae2f_restrict	h_cache;
	aclsym_fn_t* ae2f_restrict	base;
	aclspv_wrdcount_t		i;

	h_cache = aclsym_fn_cache(p->m_ctx);

	ae2f_ifezerr(h_cache)		return p->m_ctx->m_err;

	base = (aclsym_fn_t*)h_cache->m_fnlist.m_fn.m_p;

	unless(base)			return p->m_ctx->m_err;

	for(i = 0; i < h_cache->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict	p_sym = &base[i];

		/** nothing owed, or already out / turned into an import. a cursor
		 * that was never set has kind 0; clang_Cursor_isNull does not say so
		 * for a struct the cache zeroed. */
		if(!p_sym->m_body.kind || clang_Cursor_isNull(p_sym->m_body))
			continue;

		if(p_sym->m_flag0 & (aclspv_wrd_t)(
					aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED)
					| aclsym_fn_flag0(ACLSYM_FN_FLAG0_IMPORTED))) {
			continue;
		}

		/** this walk may already have queued it (the source re-defines the
		 * body, or the cursor came from this very pass). */
		{
			aclspv_wrd_t	j;
			aclspv_bool_t	c_seen = 0;

			for(j = 0; j < p->m_num; ++j) {
				if(clang_equalCursors(
							((struct s_pending*)p->m_buf->m_p)[j].m_body
							, p_sym->m_body)) {
					c_seen = 1;
					break;
				}
			}

			if(c_seen)
				continue;
		}

		if(s_push(p, p_sym->m_fn, p_sym->m_body, 0))
			return p->m_ctx->m_err;
	}

	return p->m_ctx->m_err;
}

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
		 * still collected (s_settled decides).
		 *
		 * DECLARED is not EMITTED: a forward declaration gave the function an
		 * id so its callers could be carved first, and this body is exactly
		 * what still has to go out. */
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

	/** an id without a body is NOT settled: a forward declaration exists
	 * precisely so a call site can be resolved early, and the body still has
	 * to be carved. only a body that is out, or an import the module carries
	 * instead of a body, answers for this function. */
	if(p_sym->m_flag0 & (aclspv_wrd_t)(
				aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED)
				| aclsym_fn_flag0(ACLSYM_FN_FLAG0_IMPORTED))) {
		*wr_done = 1;
		return 1;
	}

	return 0;
}

/** declare the imports every queued body needs, and prepend the declaration
 * words before the definitions already in the module.
 *
 * an import is a callee with no body anywhere in this module: the module
 * carries its declaration plus `LinkageAttributes Import` and resolves it at
 * link time.
 *
 * SPIR-V requires every function declaration to precede every definition, and
 * earlier passes may already have written definitions, so the words are
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
		aclspv_wrd_t* ae2f_restrict base;

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

/** the body index a callee emits, or c_num when the callee has no body queued
 * here (an import, a builtin, a body a later pass brings).
 *
 * matched by the SYMBOL, not by the cursor: libclang hands out a distinct
 * cursor for the call's reference, for the prototype, and for the definition,
 * and `clang_equalCursors` says they differ. the symbol cache is what already
 * unifies them, so it is what the graph is keyed on. */
static aclspv_wrd_t s_callee_slot(
		const struct s_pending*	items,
		aclspv_wrd_t		c_num,
		CXCursor		c_callee,
		h_aclspv_ctx_t		h_ctx
		) {
	aclsym_fn_t* ae2f_restrict	p_sym;
	aclspv_wrd_t			j;

	p_sym = aclsym_fn_find(c_callee, h_ctx);

	unless(p_sym)
		return c_num;

	for(j = 0; j < c_num; ++j) {
		if(items[j].m_entry)
			continue;

		if(aclsym_fn_find(items[j].m_fn, h_ctx) == p_sym)
			return j;
	}

	return c_num;
}

struct s_edge_ctx {
	const struct s_pending*		m_items;
	aclspv_wrd_t			m_num;
	aclspv_wrd_t* ae2f_restrict	m_adj;	/** c_num * c_num cells, one byte each */
	aclspv_wrd_t			m_from;
	h_aclspv_ctx_t			m_ctx;
};

static enum CXChildVisitResult s_edge_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_client
		) {
	struct s_edge_ctx* ae2f_restrict	p = rdwr_client;
	aclspv_wrd_t				c_to;

	if(c_cur.kind != CXCursor_CallExpr)
		return CXChildVisit_Recurse;

	if(aclemit_sync_is_barrier(c_cur))
		return CXChildVisit_Recurse;

	c_to = s_callee_slot(
			p->m_items
			, p->m_num
			, clang_getCursorReferenced(c_cur)
			, p->m_ctx);

	if(c_to < p->m_num)
		p->m_adj[p->m_from * p->m_num + c_to] = 1;

	return CXChildVisit_Recurse;
}

/** depth-first search from c_idx; true when a back edge closes a cycle. */
static aclspv_bool_t s_edge_dfs(
		aclspv_wrd_t			c_idx,
		aclspv_wrd_t			c_num,
		const aclspv_wrd_t* ae2f_restrict rd_adj,
		aclspv_wrd_t* ae2f_restrict	rw_state
		) {
	aclspv_wrd_t j;

	rw_state[c_idx] = 1;

	for(j = 0; j < c_num; ++j) {
		if(!rd_adj[c_idx * c_num + j])
			continue;

		/** still on the stack: this edge goes back into the current path. */
		if(rw_state[j] == 1)
			return 1;

		if(rw_state[j] == 0
				&& s_edge_dfs(j, c_num, rd_adj, rw_state))
			return 1;
	}

	rw_state[c_idx] = 2;
	return 0;
}

/**
 * is the call graph of the queued bodies cyclic?
 *
 * SPIR-V has no recursive OpFunctionCall: the call graph of a module must be
 * acyclic (see aclang.adoc, "recursion, direct or mutual"). the dependency
 * order used to catch this on its own — a cycle simply never became ready —
 * but forward declarations now let every body be carved in any order, so the
 * cycle has to be found instead of waited for.
 *
 * the graph is the one this walk can see: bodies of the main file, with an edge
 * for each call that resolves to another one of them. a callee with no body
 * here is an import and cannot close a cycle.
 *
 * `rw_adj` and `rw_state` are the caller's scratch: c_num * c_num bytes and
 * c_num bytes.
 * */
static aclspv_bool_t s_cycle(
		struct s_pending*	items,
		aclspv_wrd_t		c_num,
		aclspv_wrd_t* ae2f_restrict	rw_adj,
		aclspv_wrd_t* ae2f_restrict	rw_state,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_wrd_t i;
	aclspv_bool_t c_cyclic = 0;

	memset(rw_adj, 0, (size_t)(c_num * c_num) * sizeof(aclspv_wrd_t));
	memset(rw_state, 0, (size_t)c_num * sizeof(aclspv_wrd_t));

	for(i = 0; i < c_num; ++i) {
		struct s_edge_ctx E;

		if(items[i].m_entry)
			continue;

		E.m_items	= items;
		E.m_num		= c_num;
		E.m_adj		= rw_adj;
		E.m_from	= i;
		E.m_ctx		= h_ctx;

		clang_visitChildren(items[i].m_body, s_edge_visit, &E);
	}

	for(i = 0; i < c_num; ++i) {
		if(items[i].m_entry || rw_state[i])
			continue;

		if(s_edge_dfs(i, c_num, rw_adj, rw_state)) {
			c_cyclic = 1;
			break;
		}
	}

	(void)h_ctx;
	return c_cyclic;
}

/** give every body of this run that is still waiting an id, without writing
 * anything to the module.
 *
 * SPIR-V resolves an OpFunctionCall by id, and the definition carrying that id
 * may come later — in this pass, in a later pass, or in a later translation
 * unit. a declaration is NOT required; only the id is. so a body whose callee
 * is defined somewhere in this module but not out yet can be carved right now,
 * with the callee's id reserved for whichever pass emits its body.
 *
 * that is what unblocks:
 *
 *  - a cycle inside one translation unit (ALLOW_RECURSION),
 *  - a body whose callee lives in a translation unit that has not been
 *    compiled yet,
 *  - a body whose callee is defined later in this same translation unit.
 *
 * aclemit_body_dangling below is the other side of the bargain: a reserved id
 * whose body never arrives must be reported, not silently left undefined.
 *
 * returns how many ids were assigned.
 * */
static aclspv_compile_t s_reserve_ids(
		h_aclspv_ctx_t		h_ctx,
		struct s_pending*	items,
		aclspv_wrd_t		c_num,
		aclspv_wrd_t* ae2f_restrict	wr_made
		) {
	aclspv_wrd_t i;

	*wr_made = 0;

	for(i = 0; i < c_num; ++i) {
		aclsym_fn_t* ae2f_restrict	p_sym;

		/** an entry point has no symbol and nothing calls it. */
		if(items[i].m_entry || items[i].m_done)
			continue;

		p_sym = aclsym_fn_find(items[i].m_fn, h_ctx);

		unless(p_sym)
			continue;

		/** already nameable: emitted, imported, or reserved earlier. */
		if(p_sym->m_flag0 & ACLSYM_FN_FLAG0_CALLABLE)
			continue;

		/** a callee with NO body anywhere is an import, and the build phase
		 * declares it. reserving an id for it here would promise a definition
		 * this module does not have. */
		if(!(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)))
			continue;

		if(aclemit_fndecl_reserve(p_sym, h_ctx))
			return h_ctx->m_err;

		++*wr_made;
	}

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

	/** the bodies an earlier source deferred belong to this queue too: their
	 * callee may be one this source defines. */
	ae2f_ifnezerr(s_collect_deferred(&col)) {
		aclmem_fini(&buf);
		return h_ctx->m_err;
	}

	aclctx_err_check(h_ctx) {
		aclmem_fini(&buf);
		return h_ctx->m_err;
	}

	items = buf.m_p;
	c_num = col.m_num;



	/** a call cycle, and what to do about it.
	 *
	 * SPIR-V itself has nothing against recursion: a module with a recursive
	 * OpFunctionCall validates. VULKAN does — an entry point may not have a
	 * call graph with a cycle
	 * ([VUID-StandaloneSpirv-None-04634]), and a module built for Vulkan that
	 * recurses is rejected by spirv-val at the target environment.
	 *
	 * so the answer is the caller's, not this module's:
	 * ACLSPV_CTX_FLAG0_ALLOW_RECURSION says the target tolerates it. off (the
	 * default, and what a Vulkan module needs) the cycle is rejected before
	 * anything is emitted — otherwise forward declarations would let every
	 * body of the cycle be carved. asked here rather than by the dependency
	 * order, because forward declarations removed the "a cycle never becomes
	 * ready" signal.
	 *
	 * with the flag on, the queue's forward declarations carry the cycle
	 * through and the module is emitted as asked. */
	if(c_num && !(h_ctx->m_usrcfg.m_flag0
				& ACLSPV_CTX_FLAG0_ALLOW_RECURSION)) {
		x_aclspv_vec			scratch;
		aclspv_wrd_t* ae2f_restrict	p_adj;
		aclspv_wrd_t* ae2f_restrict	p_state;
		aclspv_bool_t			c_cyclic;

		/** the graph is c_num * c_num cells plus one state per body: kept in
		 * its own vector so the item buffer is never moved under `items`. */
		aclmem_init(&scratch, h_ctx);

		ae2f_ifnezerr(s_aclmem_cgrow2(
					&scratch
					, c_num * c_num + c_num
					, sizeof(aclspv_wrd_t)
					, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION)) {
			aclmem_fini(&scratch);
			aclmem_fini(&buf);
			return h_ctx->m_err;
		}

		p_adj	= (aclspv_wrd_t*)scratch.m_p;
		p_state	= p_adj + (c_num * c_num);

		c_cyclic = s_cycle(items, c_num, p_adj, p_state, h_ctx);


		aclmem_fini(&scratch);

		if(c_cyclic) {
			err_puts("a call cycle: Vulkan refuses a recursive call graph "
					"([VUID-StandaloneSpirv-None-04634]); set "
					"ACLSPV_CTX_FLAG0_ALLOW_RECURSION if the target accepts it");
			_aclctx_err(h_ctx, ACLSPV_COMPILE_RECURSIVE_CALL);
			aclmem_fini(&buf);
			return h_ctx->m_err;
		}
	}

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

				unless(s_ready(&items[i], h_ctx)) {
					continue;
				}

				ae2f_ifnezerr(s_emit(&items[i], h_ctx)) {
					aclmem_fini(&buf);
					return h_ctx->m_err;
				}

				items[i].m_done = 1;
				c_progress = 1;
			}
		}
		/** the queue stalled.
		 *
		 * without ALLOW_RECURSION that has exactly one cause left — a callee
		 * with no body here, which is an import and the build phase's job —
		 * because s_cycle already rejected anything that could not drain.
		 *
		 * with the flag on, a stalled queue may be a cycle the caller asked
		 * to keep: forward-declaring the bodies of this queue gives each of
		 * them an id, so the next round can carve them in any order. */
		if(!c_progress && (h_ctx->m_usrcfg.m_flag0
					& ACLSPV_CTX_FLAG0_ALLOW_RECURSION)) {
			aclspv_wrd_t c_made = 0;

			ae2f_ifnezerr(s_reserve_ids(h_ctx, items, c_num, &c_made)) {
				aclmem_fini(&buf);
				return h_ctx->m_err;
			}

			c_progress = (aclspv_bool_t)(c_made != 0);
		}
	} while(c_progress);

	/** hand what is still owed to the symbol cache.
	 *
	 * a body that is not out yet and is not an import waits for a callee only
	 * another source can bring, so the cache keeps its cursors: the next
	 * compile picks the body up again (s_collect_deferred) and carves it once
	 * the callee is out. that is what makes mutual recursion work across
	 * translation units, where no single walk sees both bodies.
	 *
	 * the cursors stay valid because the caller keeps every translation unit
	 * alive until build. */
	for(i = 0; i < c_num; ++i) {
		aclsym_fn_t* ae2f_restrict	p_sym;

		if(items[i].m_entry)
			continue;

		p_sym = aclsym_fn_find(items[i].m_fn, h_ctx);

		unless(p_sym)
			continue;

		if(items[i].m_done) {
			/** the module carries it now: nothing is owed. */
			p_sym->m_body = clang_getNullCursor();
			continue;
		}

		/** still owed: carry the cursor.
		 *
		 * whether the callee turns out to be an import or a body another
		 * source defines is not knowable from this walk — `aclsym_body_needs_import`
		 * only answers "is it in THIS source", and a later source may define
		 * it. so the body is carried either way, and dropped later by whoever
		 * settles it: the build phase when it declares the import, this pass
		 * when it emits the body. */
		p_sym->m_fn	= items[i].m_fn;
		p_sym->m_body	= items[i].m_body;
	}

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
