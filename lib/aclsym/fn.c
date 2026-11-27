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

#include "./fn.h"
#include <aclctx/err.h>

#include <aclattr/execmodel.h>
#include <aclattr/pure.h>
#include <aclattr/export.h>
#include <aclmem.h>

#include <aclemit/sync.h>
#include <aclty.h>
#include <aclutil/fn.h>

#include <ae2f/pat/dbg.h>
#include <ae2f/cc/tail.h>

#include <string.h>

/** the context pass 1 works in: the scratch sub-context, when the host has
 * one. building a signature builds an aclty type, and that must not reach
 * the module being emitted. */
static x_aclspv_ctx* s_cache(h_aclspv_ctx_t h_ctx) {
	return (h_ctx && h_ctx->m_ctx_slave) ? h_ctx->m_ctx_slave : h_ctx;
}

x_aclspv_ctx* aclsym_fn_cache(h_aclspv_ctx_t h_ctx) {
	return s_cache(h_ctx);
}

/** the symbol name of c_cur: mangled when clang has one, spelled
 * otherwise. `wr_name` owns it until disposed. */
const char* aclsym_cursor_name(CXCursor c_cur, CXString* ae2f_restrict wr_name) {
	const char* c_name;

	*wr_name = clang_Cursor_getMangling(c_cur);
	c_name = clang_getCString(*wr_name);

	if(c_name && *c_name)
		return c_name;

	/** no mangling for this entity: fall back to its source name. */
	clang_disposeString(*wr_name);
	*wr_name = clang_getCursorSpelling(c_cur);
	return clang_getCString(*wr_name);
}

/** the name arena a symbol's slices belong to. */
static const char* s_name_base(const aclsym_fn_t* c_sym, h_aclspv_ctx_t h_ctx) {
	return (c_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_STATIC))
		? h_ctx->m_fnlist.m_fn_static_name.m_p
		: h_ctx->m_fnlist.m_fn_name.m_p;
}

/** the stored name of c_sym equals [c_name, c_name + c_len). */
static aclspv_bool_t s_name_eq(
		const aclsym_fn_t*	c_sym,
		const char*		c_name,
		aclspv_wrd_t		c_len,
		h_aclspv_ctx_t		h_ctx
		) {
	const char* p_base;

	if(!c_sym || !c_name || c_sym->m_len_name != c_len)
		return 0;

	p_base = s_name_base(c_sym, h_ctx);

	if(!p_base)
		return 0;

	return (aclspv_bool_t)!memcmp(
			p_base + c_sym->m_idx_name
			, c_name
			, (size_t)c_len
			);
}

/** the stored USR of c_sym equals [c_usr, c_usr + c_len). */
static aclspv_bool_t s_usr_eq(
		const aclsym_fn_t*	c_sym,
		const char*		c_usr,
		aclspv_wrd_t		c_len,
		h_aclspv_ctx_t		h_ctx
		) {
	const char* p_base;

	if(!c_sym || !c_usr || c_sym->m_len_usr != c_len)
		return 0;

	p_base = s_name_base(c_sym, h_ctx);

	if(!p_base)
		return 0;

	return (aclspv_bool_t)!memcmp(
			p_base + c_sym->m_idx_usr
			, c_usr
			, (size_t)c_len
			);
}

/** append a text slice to an arena; the symbol keeps the slice. */
static aclspv_compile_t s_txt_app(
		const char*		c_txt,
		aclspv_wrd_t* ae2f_restrict	wr_idx,
		aclspv_wrd_t* ae2f_restrict	wr_len,
		x_aclspv_vec* ae2f_restrict	h_arena,
		aclspv_wrdcount_t* ae2f_restrict	h_num,
		h_aclspv_ctx_t		h_ctx
		) {
	size_t			c_len = c_txt ? strlen(c_txt) : 0;
	const size_t		c_len_term = c_len + 1;	/** null included */
	aclspv_compile_t	ret = ACLSPV_COMPILE_OK;

	(void)h_ctx;

	*wr_idx = *h_num;
	*wr_len = (aclspv_wrd_t)c_len;

	if(s_aclmem_cgrow2(
				h_arena
				, *h_num + (aclspv_wrd_t)c_len_term
				, 1
				, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
		ret = h_arena->m_ctx->m_err;
	else {
		memcpy(
				(char*)h_arena->m_p + *wr_idx
				, c_txt ? c_txt : ""
				, c_len_term
				);
		*h_num += (aclspv_wrd_t)c_len_term;
	}

	return ret;
}

/** append c_fn's symbol name to `h_arena` and keep the slice. */
static aclspv_compile_t s_name_app(
		CXCursor		c_fn,
		x_aclspv_vec* ae2f_restrict	h_arena,
		aclspv_wrdcount_t* ae2f_restrict	h_num,
		aclspv_wrd_t* ae2f_restrict	wr_idx,
		aclspv_wrd_t* ae2f_restrict	wr_len,
		h_aclspv_ctx_t		h_ctx
		) {
	CXString		NAME;
	const char*		c_name = aclsym_cursor_name(c_fn, &NAME);
	aclspv_compile_t	ret = s_txt_app(c_name, wr_idx, wr_len, h_arena, h_num, h_ctx);

	clang_disposeString(NAME);
	return ret;
}

/** append c_fn's USR to `h_arena` and keep the slice. */
static aclspv_compile_t s_usr_app(
		CXCursor		c_fn,
		x_aclspv_vec* ae2f_restrict	h_arena,
		aclspv_wrdcount_t* ae2f_restrict	h_num,
		aclspv_wrd_t* ae2f_restrict	wr_idx,
		aclspv_wrd_t* ae2f_restrict	wr_len,
		h_aclspv_ctx_t		h_ctx
		) {
	CXString		USR;
	const char*		c_usr;
	aclspv_compile_t	ret;

	USR = clang_getCursorUSR(c_fn);
	c_usr = clang_getCString(USR);
	ret = s_txt_app(c_usr, wr_idx, wr_len, h_arena, h_num, h_ctx);

	clang_disposeString(USR);
	return ret;
}

/** internal-linkage declarations live in their own cache. */
static aclspv_bool_t s_is_static(CXCursor c_fn) {
	return (aclspv_bool_t)(clang_getCursorLinkage(c_fn) == CXLinkage_Internal);
}

/** the cache the declaration belongs to. */
static x_aclspv_vec* s_fnvec(h_aclspv_ctx_t h_ctx, aclspv_bool_t c_static) {
	return c_static
		? &h_ctx->m_fnlist.m_fn_static
		: &h_ctx->m_fnlist.m_fn;
}

/** the number of entries in s_fnvec(). */
static aclspv_wrdcount_t* s_fncount(h_aclspv_ctx_t h_ctx, aclspv_bool_t c_static) {
	return c_static
		? &h_ctx->m_fnlist.m_num_fn_static
		: &h_ctx->m_fnlist.m_num_fn;
}

/** the normal symbol named c_fn's mangled name. */
static aclsym_fn_t* s_find_normal(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	CXString		NAME;
	const char*		c_name = aclsym_cursor_name(c_fn, &NAME);
	aclspv_wrd_t		c_len = c_name ? (aclspv_wrd_t)strlen(c_name) : 0;
	aclspv_wrdcount_t	i;
	aclsym_fn_t* ae2f_restrict	r = 0;

	for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)[i];

		if(s_name_eq(p, c_name, c_len, h_ctx)) {
			r = p;
			break;
		}
	}

	clang_disposeString(NAME);

	return r;
}

/** the static symbol for c_fn: USR first, mark second, name last. */
static aclsym_fn_t* s_find_static(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	CXString		USR;
	CXString		NAME;
	const char*		c_usr;
	const char*		c_name;
	aclspv_wrd_t		c_len_usr;
	aclspv_wrd_t		c_len_name;
	aclspv_wrd_t		c_mark;
	aclspv_wrdcount_t	i;
	aclsym_fn_t* ae2f_restrict	r = 0;
	aclspv_bool_t		c_has_usr;

	USR = clang_getCursorUSR(c_fn);
	c_usr = clang_getCString(USR);
	c_has_usr = (aclspv_bool_t)(c_usr && *c_usr);
	c_len_usr = c_usr ? (aclspv_wrd_t)strlen(c_usr) : 0;
	c_mark = (aclspv_wrd_t)clang_hashCursor(c_fn);

	for(i = 0; c_has_usr && i < h_ctx->m_fnlist.m_num_fn_static; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn_static.m_p)[i];

		if(s_usr_eq(p, c_usr, c_len_usr, h_ctx)) {
			r = p;
			break;
		}
	}

	clang_disposeString(USR);

	if(r)
		return r;

	/** a non-empty USR that matched nothing is a distinct static entity,
	 * even when another one shares its mangled/display name. keeping them
	 * apart is the whole reason this cache is keyed by USR. */
	if(c_has_usr)
		return 0;

	/** no USR at all: the cursor hash is still a useful shortcut for the
	 * common declaration-to-definition lookup. */
	for(i = 0; i < h_ctx->m_fnlist.m_num_fn_static; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn_static.m_p)[i];

		if(p->m_mark == c_mark)
			return p;
	}

	NAME = clang_Cursor_getMangling(c_fn);
	c_name = clang_getCString(NAME);

	if(!c_name || !*c_name) {
		clang_disposeString(NAME);
		NAME = clang_getCursorSpelling(c_fn);
		c_name = clang_getCString(NAME);
	}

	c_len_name = c_name ? (aclspv_wrd_t)strlen(c_name) : 0;

	for(i = 0; i < h_ctx->m_fnlist.m_num_fn_static; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn_static.m_p)[i];

		if(s_name_eq(p, c_name, c_len_name, h_ctx)) {
			r = p;
			break;
		}
	}

	clang_disposeString(NAME);
	return r;
}

/** true when c_fn carries aclspv_execmodel. */
static aclspv_bool_t s_is_entry(CXCursor c_fn) {
	enum SpvExecutionModel_ MODEL = SpvExecutionModelMax;

	clang_visitChildren(c_fn, s_attr_execmodel, &MODEL);

	return (aclspv_bool_t)(MODEL != SpvExecutionModelMax);
}

/** the aclty id of c_fn's declared signature.
 *
 * the DECLARED type is passed on purpose: canonicalising here would drop the
 * typedef sugar, and the storage-class / layout annotations this compiler
 * lives by are attached to the typedef, not to the clang type. aclty is the
 * layer that understands them, and it interns by structure, so spelling
 * variants of the same signature still land on one id.
 *
 * the type is entered TWICE, once per context. the cache (slave) keeps the
 * comparison key m_type_id; the module context keeps the real entry the build
 * phase emits from. both are interned by structure, so they agree, and
 * neither has to be rebuilt later from a cursor that may be gone. */
static aclty_subid_t s_type(
		CXCursor		c_fn,
		h_aclspv_ctx_t		h_cache,
		h_aclspv_ctx_t		h_module
		) {
	const CXType		TY = clang_getCursorType(c_fn);
	const aclspv_id_t	ID = aclty_mk_fn1_this(c_fn, TY, h_cache);

	ae2f_ifezerr(ID)
		return 0;

	/** the module half: the same declared type, interned in the context that
	 * writes the module. a build phase that takes no source finds it there
	 * and never has to rebuild it from a cursor. */
	if(h_module && h_module != h_cache) {
		ae2f_ifezerr(aclty_mk_fn1_this(c_fn, TY, h_module))
			return 0;
	}

	return (aclty_subid_t)ID;
}

aclsym_fn_t* aclsym_fn_find(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	CXString		NAME;
	const char*		c_name;
	aclspv_wrd_t		c_len;
	aclspv_wrd_t		c_mark;
	aclspv_wrdcount_t	i;
	aclspv_bool_t		c_static;

	ae2f_ifezerr(h_ctx)			return 0;

	h_ctx = s_cache(h_ctx);

	c_static = s_is_static(c_fn);

	if(c_static)
		return s_find_static(c_fn, h_ctx);

	ae2f_ifezerr(h_ctx->m_fnlist.m_fn.m_p)	return 0;

	c_name	= aclsym_cursor_name(c_fn, &NAME);
	c_len	= c_name ? (aclspv_wrd_t)strlen(c_name) : 0;
	c_mark	= (aclspv_wrd_t)clang_hashCursor(c_fn);

	/** exact mark first: the common same-declaration case. */
	for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)[i];

		if(p->m_mark == c_mark && s_name_eq(p, c_name, c_len, h_ctx)) {
			clang_disposeString(NAME);
			return p;
		}
	}

	/** clang hands out a distinct cursor for a prototype use vs the
	 * definition: the name is the identity, the mark is only a shortcut. */
	{
		aclsym_fn_t* ae2f_restrict r = s_find_normal(c_fn, h_ctx);
		clang_disposeString(NAME);
		return r;
	}
}

aclsym_fn_t* aclsym_fn_find_id(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	aclspv_wrdcount_t i;

	if(!c_id)		return 0;
	ae2f_ifezerr(h_ctx)	return 0;

	h_ctx = s_cache(h_ctx);

	for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)[i];

		if(p->m_id == c_id)
			return p;
	}

	for(i = 0; i < h_ctx->m_fnlist.m_num_fn_static; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn_static.m_p)[i];

		if(p->m_id == c_id)
			return p;
	}

	return 0;
}

static aclspv_compile_t s_add(
		CXCursor		c_fn,
		h_aclspv_ctx_t		h_ctx,
		h_aclspv_ctx_t		h_module
		) {
	aclsym_fn_t* ae2f_restrict	p_sym;
	const aclspv_bool_t		c_static	= s_is_static(c_fn);
	const aclspv_bool_t		is_def		= (aclspv_bool_t)clang_isCursorDefinition(c_fn);
	const aclattr_pure_t		c_pure		= aclattr_get_pure(c_fn);
	const aclspv_wrd_t		c_mark		= (aclspv_wrd_t)clang_hashCursor(c_fn);
	aclty_subid_t			c_ty;
	x_aclspv_vec* ae2f_restrict	p_list;
	aclspv_wrdcount_t*		p_num;
	x_aclspv_vec* ae2f_restrict	p_arena;
	aclspv_wrdcount_t*		p_name_num;

	p_list		= s_fnvec(h_ctx, c_static);
	p_num		= s_fncount(h_ctx, c_static);
	p_arena		= c_static
		? &h_ctx->m_fnlist.m_fn_static_name
		: &h_ctx->m_fnlist.m_fn_name;
	p_name_num	= c_static
		? &h_ctx->m_fnlist.m_num_fn_static_name
		: &h_ctx->m_fnlist.m_num_fn_name;

	c_ty = s_type(c_fn, h_ctx, h_module);

	if(!c_ty) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return h_ctx->m_err;
	}

	p_sym = c_static
		? s_find_static(c_fn, h_ctx)
		: s_find_normal(c_fn, h_ctx);

	if(p_sym) {
		/** one name, one declared function type. the aclty arena interns
		 * types, so comparing the ids compares the signatures. */
		if(p_sym->m_type_id != c_ty) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_MET_INVAL_FN_REDECL);
			return h_ctx->m_err;
		}

		/** a definition supersedes a prototype: its mark is the handle the
		 * build phase will resolve the body through. */
		if(is_def && !(p_sym->m_flag0
					& aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED))) {
			p_sym->m_mark	= c_mark;
			p_sym->m_flag0	|= aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED);
		}

		if(c_static)
			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_STATIC);
	}
	else {
		aclspv_wrd_t	c_idx_name;
		aclspv_wrd_t	c_len_name;
		aclspv_wrd_t	c_idx_usr = 0;
		aclspv_wrd_t	c_len_usr = 0;

		ae2f_ifnezerr(s_name_app(
					c_fn
					, p_arena
					, p_name_num
					, &c_idx_name
					, &c_len_name
					, h_ctx))
			return h_ctx->m_err;

		if(c_static) {
			ae2f_ifnezerr(s_usr_app(
						c_fn
						, p_arena
						, p_name_num
						, &c_idx_usr
						, &c_len_usr
						, h_ctx))
				return h_ctx->m_err;
		}

		ae2f_ifnezerr(s_aclmem_cgrow2(
					p_list
					, *p_num + 1
					, sizeof(aclsym_fn_t)
					, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
			return h_ctx->m_err;

		p_sym = &((aclsym_fn_t*)p_list->m_p)[*p_num];

		aclmemzero(p_sym, sizeof(*p_sym));

		p_sym->m_mark		= c_mark;
		p_sym->m_type_id	= c_ty;
		p_sym->m_idx_name	= c_idx_name;
		p_sym->m_len_name	= c_len_name;
		p_sym->m_idx_usr	= c_idx_usr;
		p_sym->m_len_usr	= c_len_usr;
		p_sym->m_flag0		= is_def
			? aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)
			: (aclspv_wrd_t)0;

		if(c_static)
			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_STATIC);

		++*p_num;
	}

	/** the declared promise only ever accumulates: it may sit on any of the
	 * redeclarations. a body later replaces it with the analysed answer. */
	if(c_pure.m_is_pure)
		p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE);
	if(c_pure.m_is_const)
		p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST);

	/** the export intent is a property of the declaration, recorded here while
	 * the translation unit can still be asked: the build phase decorates from
	 * the symbol, and a build that takes no source has no cursor left to read
	 * it from. like the purity promise, it only accumulates. */
	{
		const aclattr_export_t EXP = aclattr_get_export(c_fn);

		if(EXP.m_export)
			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_DECL_EXPORT);
		if(EXP.m_hidden)
			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_DECL_HIDDEN);
	}

	return ACLSPV_COMPILE_OK;
}

/** record an entry-point declaration: no signature, no import, no export.
 *
 * the symbol exists only so a later phase can tell, without the translation
 * unit, that this entry point's body has not been emitted yet. it is made
 * once and re-marked on every pass that still sees the declaration. */
static aclspv_compile_t s_add_entry(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	const aclspv_wrd_t	c_mark = (aclspv_wrd_t)clang_hashCursor(c_fn);
	aclsym_fn_t* ae2f_restrict	p_sym;
	aclspv_wrdcount_t		i;

	/** matched by mark, never by name: a normal symbol that happens to share
	 * the name is a different entity (the program would not link, and clang
	 * has already said so), and the name cache is full of symbols from
	 * translation units that are gone. */
	for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)[i];

		if(p->m_mark == c_mark) {
			p->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY);
			return ACLSPV_COMPILE_OK;
		}
	}

	for(i = 0; i < h_ctx->m_fnlist.m_num_fn_static; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn_static.m_p)[i];

		if(p->m_mark == c_mark) {
			p->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY);
			return ACLSPV_COMPILE_OK;
		}
	}

	ae2f_ifnezerr(s_aclmem_cgrow2(
				&h_ctx->m_fnlist.m_fn,
				h_ctx->m_fnlist.m_num_fn + 1,
				sizeof(aclsym_fn_t),
				ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
		return h_ctx->m_err;

	p_sym = &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)
		[h_ctx->m_fnlist.m_num_fn];

	aclmemzero(p_sym, sizeof(*p_sym));

	p_sym->m_mark		= c_mark;
	p_sym->m_flag0		= aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)
		| aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY);

	++h_ctx->m_fnlist.m_num_fn;

	return ACLSPV_COMPILE_OK;
}

/** what the gather walk carries: the symbol cache it writes, and the module
 * context a declared signature is also interned in (see s_type). */
struct s_gather {
	h_aclspv_ctx_t	m_cache;
	h_aclspv_ctx_t	m_module;
};

static enum CXChildVisitResult s_gather(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_data
		) {
	const struct s_gather* ae2f_restrict	data = rdwr_data;
	h_aclspv_ctx_t				ctx = data->m_cache;

	/** declarations only: never descend into a body. records and
	 * namespaces are containers, so their methods are still declarations. */
	switch((aclspv_cxcurkind_t)c_cur.kind) {
		case CXCursor_FunctionDecl:
			if(aclutil_fn_is_std_move_decl(c_cur))
				return CXChildVisit_Continue;
			break;

		case CXCursor_CXXMethod:
		case CXCursor_Constructor:
		case CXCursor_Destructor:
			break;

		case CXCursor_Namespace:
		case CXCursor_StructDecl:
		case CXCursor_ClassDecl:
		case CXCursor_UnionDecl:
		case CXCursor_LinkageSpec:
			return CXChildVisit_Recurse;

		default:
			return CXChildVisit_Continue;
	}

	unless(clang_Location_isFromMainFile(clang_getCursorLocation(c_cur)))
		return CXChildVisit_Continue;

	if(s_is_entry(c_cur)) {
		/** an entry point is not a call target: neither another entry point
		 * nor a normal function may call it. give it no type, so nothing of
		 * its signature reaches the module, and mark it ENTRY so the symbol is
		 * not NAMED: no call resolves to it and no export decorates it.
		 *
		 * the record itself is still made: it is what tells a build that takes
		 * no source whether this entry point's body has gone out yet. its body
		 * is parsed as an entry point (see aclemit_fndecl_entp). */
		ae2f_ifnezerr(s_add_entry(c_cur, ctx))
			return CXChildVisit_Break;

		return CXChildVisit_Continue;
	}

	ae2f_ifnezerr(s_add(c_cur, ctx, data->m_module))
		return CXChildVisit_Break;

	return CXChildVisit_Continue;
}

aclspv_compile_t aclsym_fn_gather(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	struct s_gather	data;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	data.m_cache	= s_cache(h_ctx);
	/** the module context the types are interned in as well: h_ctx itself. */
	data.m_module	= h_ctx;

	clang_visitChildren(c_root, s_gather, &data);

	/** a failure inside the scratch context is the compile's failure. */
	if(data.m_cache->m_err && !h_ctx->m_err)
		_aclctx_err(h_ctx, data.m_cache->m_err);

	return h_ctx->m_err;
}

const char* aclsym_fn_name(const aclsym_fn_t* c_sym, h_aclspv_ctx_t h_ctx) {
	const char*	c_base;
	aclspv_wrdcount_t c_num_name;

	ae2f_ifezerr(c_sym)	return 0;

	h_ctx = s_cache(h_ctx);

	ae2f_ifezerr(h_ctx)	return 0;

	if(c_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_STATIC)) {
		c_base		= h_ctx->m_fnlist.m_fn_static_name.m_p;
		c_num_name	= h_ctx->m_fnlist.m_num_fn_static_name;
	}
	else {
		c_base		= h_ctx->m_fnlist.m_fn_name.m_p;
		c_num_name	= h_ctx->m_fnlist.m_num_fn_name;
	}

	ae2f_ifezerr(c_base)			return 0;
	ae2f_ifnezerr_strict(c_sym->m_idx_name >= c_num_name)
		return 0;

	return c_base + c_sym->m_idx_name;
}

/** true when the symbol still describes this pass: the module carries it, as a
 * body or as an import. everything else is a claim about a translation unit
 * that has since died.
 *
 * DEFINED alone is not enough. a body another translation unit declared but
 * never emitted is a cursor into that unit — nothing in the module answers for
 * it, and the pass starting now has to see the declaration again. only EMITTED
 * (the body is out) and IMPORTED (the module declares it) are facts about the
 * module rather than about a source. */
static aclspv_bool_t s_kept(const aclsym_fn_t* c_sym) {
	return (aclspv_bool_t)(c_sym->m_flag0 & (aclspv_wrd_t)ACLSYM_FN_FLAG0_NAMED);
}

void aclsym_fn_static_reset_pass(h_aclspv_ctx_t h_ctx) {
	aclsym_fn_t* ae2f_restrict	base;
	aclspv_wrdcount_t		c_kept = 0;
	aclspv_wrdcount_t		i;

	ae2f_ifezerr(h_ctx)		return;

	h_ctx = s_cache(h_ctx);

	ae2f_ifezerr(h_ctx)		return;

	base = (aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p;

	if(!base) {
		h_ctx->m_fnlist.m_num_fn = 0;
	}
	else {
		for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
			/** an ENTRY mark is a claim about the NEW source: the pass that
			 * is starting has to see the declaration again (s_add_entry
			 * re-marks it). the emitted or imported meaning survives. */
			base[i].m_flag0 &= ~aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY);

			if(!s_kept(&base[i]))
				continue;

			if(c_kept != i)
				base[c_kept] = base[i];

			++c_kept;
		}

		h_ctx->m_fnlist.m_num_fn = c_kept;
	}

	/** the static half is TU-local by definition: no symbol of it survives a
	 * different source, so both counts go back to zero and the buffers stay
	 * for the next pass to reuse. */
	h_ctx->m_fnlist.m_num_fn_static		= 0;
	h_ctx->m_fnlist.m_num_fn_static_name	= 0;
}

void aclsym_fn_reset_static(h_aclspv_ctx_t h_ctx) {
	ae2f_ifezerr(h_ctx)		return;

	h_ctx = s_cache(h_ctx);

	ae2f_ifezerr(h_ctx)		return;

	/** the vectors keep their buffers: only the used length goes back to
	 * zero, so the next pass reuses the same storage. */
	h_ctx->m_fnlist.m_num_fn_static		= 0;
	h_ctx->m_fnlist.m_num_fn_static_name	= 0;
}

aclspv_bool_t aclsym_fn_needs_import(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	aclsym_fn_t* ae2f_restrict	p_sym;

	p_sym = aclsym_fn_find(c_fn, h_ctx);

	/** only a body can call: a prototype has nothing to wait for. */
	if(!p_sym
			|| !(p_sym->m_flag0
				& aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)))
		return 0;

	{ ae2fcc_tail_f return aclsym_body_needs_import(c_fn, h_ctx); }
}

struct s_body_import {
	h_aclspv_ctx_t	m_ctx;
	aclspv_bool_t	m_hit;
};

static enum CXChildVisitResult s_body_import_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_client
		) {
	struct s_body_import* ae2f_restrict p = rdwr_client;
	CXCursor		c_callee;
	aclsym_fn_t* ae2f_restrict p_sym;

	if(c_cur.kind != CXCursor_CallExpr)
		return CXChildVisit_Recurse;

	/** a barrier is lowered in place: it is never imported. */
	if(aclemit_sync_is_barrier(c_cur))
		return CXChildVisit_Recurse;

	c_callee = clang_getCursorReferenced(c_cur);

	if(clang_Cursor_isNull(c_callee)
			|| (c_callee.kind != CXCursor_FunctionDecl
				&& !aclutil_fn_is_method(c_callee)))
		return CXChildVisit_Recurse;

	p_sym = aclsym_fn_find(c_callee, p->m_ctx);

	/** a builtin / header function: not a function call at all. */
	unless(p_sym)
		return CXChildVisit_Recurse;

	/** a body in this translation unit is an emission-order problem, not an
	 * import. */
	if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED))
		return CXChildVisit_Recurse;

	p->m_hit = 1;
	return CXChildVisit_Break;
}

aclspv_bool_t aclsym_body_needs_import(CXCursor c_body, h_aclspv_ctx_t h_ctx) {
	struct s_body_import p;

	p.m_ctx	= h_ctx;
	p.m_hit	= 0;

	clang_visitChildren(c_body, s_body_import_visit, &p);
	return p.m_hit;
}

struct s_body_ready {
	h_aclspv_ctx_t	m_ctx;
	aclspv_bool_t	m_ready;
};

static enum CXChildVisitResult s_body_ready_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_client
		) {
	struct s_body_ready* ae2f_restrict p = rdwr_client;
	CXCursor		c_callee;
	aclsym_fn_t* ae2f_restrict p_sym;

	if(c_cur.kind != CXCursor_CallExpr)
		return CXChildVisit_Recurse;

	/** a barrier is lowered in place: it never needs a callee. */
	if(aclemit_sync_is_barrier(c_cur))
		return CXChildVisit_Recurse;

	c_callee = clang_getCursorReferenced(c_cur);

	if(clang_Cursor_isNull(c_callee)
			|| (c_callee.kind != CXCursor_FunctionDecl
				&& !aclutil_fn_is_method(c_callee)))
		return CXChildVisit_Recurse;

	p_sym = aclsym_fn_find(c_callee, p->m_ctx);

	/** a builtin / header function: not a function call in this IR. */
	unless(p_sym)
		return CXChildVisit_Recurse;

	/** the module can name it: a body emitted by a pass, or an import the
	 * build phase already declared. */
	if(p_sym->m_flag0 & ACLSYM_FN_FLAG0_NAMED)
		return CXChildVisit_Recurse;

	/** a body-less callee becomes nameable once its import is declared;
	 * a defined callee must be emitted first. either way: not yet. */
	p->m_ready = 0;
	return CXChildVisit_Break;
}

aclspv_bool_t aclsym_body_ready(CXCursor c_body, h_aclspv_ctx_t h_ctx) {
	struct s_body_ready p;

	p.m_ctx		= h_ctx;
	p.m_ready	= 1;

	clang_visitChildren(c_body, s_body_ready_visit, &p);

	return p.m_ready;
}

aclspv_bool_t aclsym_fn_ready(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	aclsym_fn_t* ae2f_restrict	p_sym;

	p_sym = aclsym_fn_find(c_fn, h_ctx);

	/** not a body of this translation unit: an entry point (no symbol) or a
	 * prototype. nothing will be emitted for it, so nothing is waited on. */
	if(!p_sym
			|| !(p_sym->m_flag0
				& aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)))
		return 1;

	/** the same predicate as aclsym_body_ready, asked from the function
	 * cursor: every callee must already have an id in the module. */
	{ ae2fcc_tail_f return aclsym_body_ready(c_fn, h_ctx); }
}
