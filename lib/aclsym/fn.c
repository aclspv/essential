#include "./fn.h"
#include <aclctx/err.h>

#include <aclattr/execmodel.h>
#include <aclattr/pure.h>
#include <aclmem.h>

#include <aclemit/sync.h>
#include <aclty.h>

#include <ae2f/pat/dbg.h>
#include <ae2f/cc/tail.h>

#include <string.h>

/** the context pass 1 works in: the scratch sub-context, when the host has
 * one. building a signature builds an aclty type, and that must not reach
 * the module being emitted. */
static x_aclspv_ctx* s_cache(h_aclspv_ctx_t h_ctx) {
	return (h_ctx && h_ctx->m_ctx_slave) ? h_ctx->m_ctx_slave : h_ctx;
}

/** spelling of c_fn; `wr_spell` owns it until disposed. */
static const char* s_spell(CXCursor c_fn, CXString* ae2f_restrict wr_spell) {
	*wr_spell = clang_getCursorSpelling(c_fn);
	return clang_getCString(*wr_spell);
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

	p_base = h_ctx->m_fnlist.m_fn_name.m_p;

	if(!p_base)
		return 0;

	return (aclspv_bool_t)!memcmp(
			p_base + c_sym->m_idx_name
			, c_name
			, (size_t)c_len
			);
}

/** append c_fn's spelling to the name arena; the symbol keeps the slice. */
static aclspv_compile_t s_name_app(
		CXCursor		c_fn,
		aclspv_wrd_t* ae2f_restrict	wr_idx,
		aclspv_wrd_t* ae2f_restrict	wr_len,
		h_aclspv_ctx_t		h_ctx
		) {
	CXString		SPELL;
	const char*		c_name = s_spell(c_fn, &SPELL);
	size_t			c_len = c_name ? strlen(c_name) : 0;
	const size_t		c_len_term = c_len + 1;	/** null included */
	aclspv_compile_t	ret = ACLSPV_COMPILE_OK;

	*wr_idx = h_ctx->m_fnlist.m_num_fn_name;
	*wr_len = (aclspv_wrd_t)c_len;

	if(s_aclmem_cgrow2(
				&h_ctx->m_fnlist.m_fn_name
				, h_ctx->m_fnlist.m_num_fn_name
					+ (aclspv_wrd_t)c_len_term
				, 1
				, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
		ret = h_ctx->m_err;
	else {
		memcpy(
				(char*)h_ctx->m_fnlist.m_fn_name.m_p
					+ *wr_idx
				, c_name ? c_name : ""
				, c_len_term
				);
		h_ctx->m_fnlist.m_num_fn_name += (aclspv_wrd_t)c_len_term;
	}

	clang_disposeString(SPELL);

	return ret;
}

/** the symbol declared with c_fn's spelling. */
static aclsym_fn_t* s_find_spelling(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	CXString		SPELL;
	const char*		c_name = s_spell(c_fn, &SPELL);
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

	clang_disposeString(SPELL);

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
 * variants of the same signature still land on one id. */
ae2f_inline_f static aclty_subid_t s_type(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	return (aclty_subid_t)aclty_mk_fn1(
			c_fn
			, clang_getCursorType(c_fn)
			, h_ctx
			);
}

aclsym_fn_t* aclsym_fn_find(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	CXString		SPELL;
	const char*		c_name;
	aclspv_wrd_t		c_len;
	aclspv_wrd_t		c_mark;
	aclspv_wrdcount_t	i;

	ae2f_ifezerr(h_ctx)			return 0;

	h_ctx = s_cache(h_ctx);

	ae2f_ifezerr(h_ctx->m_fnlist.m_fn.m_p)	return 0;

	c_name	= s_spell(c_fn, &SPELL);
	c_len	= c_name ? (aclspv_wrd_t)strlen(c_name) : 0;
	c_mark	= (aclspv_wrd_t)clang_hashCursor(c_fn);

	/** exact mark first: the common same-declaration case. */
	for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)[i];

		if(p->m_mark == c_mark && s_name_eq(p, c_name, c_len, h_ctx)) {
			clang_disposeString(SPELL);
			return p;
		}
	}

	/** clang hands out a distinct cursor for a prototype use vs the
	 * definition: the name is the identity, the mark is only a shortcut. */
	{
		aclsym_fn_t* ae2f_restrict r = s_find_spelling(c_fn, h_ctx);
		clang_disposeString(SPELL);
		return r;
	}
}

aclsym_fn_t* aclsym_fn_find_id(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	aclspv_wrdcount_t i;

	if(!c_id)		return 0;
	ae2f_ifezerr(h_ctx)	return 0;

	h_ctx = s_cache(h_ctx);

	ae2f_ifezerr(h_ctx->m_fnlist.m_fn.m_p)	return 0;

	for(i = 0; i < h_ctx->m_fnlist.m_num_fn; ++i) {
		aclsym_fn_t* ae2f_restrict p
			= &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)[i];

		if(p->m_id == c_id)
			return p;
	}

	return 0;
}

static aclspv_compile_t s_add(CXCursor c_fn, h_aclspv_ctx_t h_ctx) {
	aclsym_fn_t* ae2f_restrict	p_sym;
	const aclspv_bool_t		is_def		= (aclspv_bool_t)clang_isCursorDefinition(c_fn);
	const aclattr_pure_t		c_pure		= aclattr_get_pure(c_fn);
	const aclspv_wrd_t		c_mark		= (aclspv_wrd_t)clang_hashCursor(c_fn);
	aclty_subid_t			c_ty;

	c_ty = s_type(c_fn, h_ctx);

	if(!c_ty) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return h_ctx->m_err;
	}

	p_sym = s_find_spelling(c_fn, h_ctx);

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
	}
	else {
		aclspv_wrd_t	c_idx_name;
		aclspv_wrd_t	c_len_name;

		ae2f_ifnezerr(s_name_app(
					c_fn
					, &c_idx_name
					, &c_len_name
					, h_ctx))
			return h_ctx->m_err;

		ae2f_ifnezerr(s_aclmem_cgrow2(
					&h_ctx->m_fnlist.m_fn
					, h_ctx->m_fnlist.m_num_fn + 1
					, sizeof(aclsym_fn_t)
					, ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION))
			return h_ctx->m_err;

		p_sym = &((aclsym_fn_t*)h_ctx->m_fnlist.m_fn.m_p)
			[h_ctx->m_fnlist.m_num_fn];

		aclmemzero(p_sym, sizeof(*p_sym));

		p_sym->m_mark		= c_mark;
		p_sym->m_type_id	= c_ty;
		p_sym->m_idx_name	= c_idx_name;
		p_sym->m_len_name	= c_len_name;
		p_sym->m_flag0		= is_def
			? aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)
			: (aclspv_wrd_t)0;

		++h_ctx->m_fnlist.m_num_fn;
	}

	/** the declared promise only ever accumulates: it may sit on any of the
	 * redeclarations. a body later replaces it with the analysed answer. */
	if(c_pure.m_is_pure)
		p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE);
	if(c_pure.m_is_const)
		p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST);

	return ACLSPV_COMPILE_OK;
}

static enum CXChildVisitResult s_gather(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_ctx
		) {
	h_aclspv_ctx_t ctx = rdwr_ctx;

	/** declarations only: never descend into a body. */
	if(c_cur.kind != CXCursor_FunctionDecl)
		return CXChildVisit_Continue;

	unless(clang_Location_isFromMainFile(clang_getCursorLocation(c_cur)))
		return CXChildVisit_Continue;

	/** an entry point is not a call target: neither another entry point nor
	 * a normal function may call it. give it no symbol and build no type,
	 * so nothing of its declaration reaches the module. its body is still
	 * parsed as an entry point (see aclemit_fndecl_entp). */
	if(s_is_entry(c_cur))
		return CXChildVisit_Continue;

	ae2f_ifnezerr(s_add(c_cur, ctx))
		return CXChildVisit_Break;

	return CXChildVisit_Continue;
}

aclspv_compile_t aclsym_fn_gather(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	h_aclspv_ctx_t cache;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	aclctx_err_check(h_ctx)	return h_ctx->m_err;

	cache = s_cache(h_ctx);

	clang_visitChildren(c_root, s_gather, cache);

	/** a failure inside the scratch context is the compile's failure. */
	if(cache->m_err && !h_ctx->m_err)
		_aclctx_err(h_ctx, cache->m_err);

	return h_ctx->m_err;
}

const char* aclsym_fn_name(const aclsym_fn_t* c_sym, h_aclspv_ctx_t h_ctx) {
	ae2f_ifezerr(c_sym)	return 0;

	h_ctx = s_cache(h_ctx);

	ae2f_ifezerr(h_ctx)			return 0;
	ae2f_ifezerr(h_ctx->m_fnlist.m_fn_name.m_p)	return 0;
	ae2f_ifnezerr_strict(c_sym->m_idx_name
			>= h_ctx->m_fnlist.m_num_fn_name)
		return 0;

	return (const char*)h_ctx->m_fnlist.m_fn_name.m_p + c_sym->m_idx_name;
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

	if(clang_Cursor_isNull(c_callee) || c_callee.kind != CXCursor_FunctionDecl)
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

	if(clang_Cursor_isNull(c_callee) || c_callee.kind != CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	p_sym = aclsym_fn_find(c_callee, p->m_ctx);

	/** a builtin / header function: not a function call in this IR. */
	unless(p_sym)
		return CXChildVisit_Recurse;

	/** the module can name it. */
	if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))
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
