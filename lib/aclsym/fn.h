/** @file aclsym/fn.h
 *
 * pre-emit function symbols. no aclcur graph, no emission: records only.
 *
 * no CXCursor is kept. a cursor is a language-precise view into a live
 * translation unit, not compiler state: the caller owns the TU's lifetime and
 * must keep it alive while the compiler is still using the cache (point the
 * compile phase at it, keep it until build). a symbol carries only a 4-byte
 * mark and its name, never the cursor itself.
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

	ACLSYM_FN_FLAG0_MAX
};

ae2f_inline_f ae2f_ccconst static aclspv_wrd_t aclsym_fn_flag0(
		enum ACLSYM_FN_FLAG0_ c
		) {
	return ((aclspv_wrd_t)1u) << (aclspv_wrd_t)c;
}


/** one function symbol: the cache pass 1 builds before any body is parsed.
 *
 * the symbol is keyed by the FunctionDecl cursor (with a spelling fallback
 * for a cursor that clang hands out twice) and covers declarations and
 * definitions alike. a definition supersedes an earlier prototype.
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
	 * no CXCursor is stored. a cursor is a language-precise view into a
	 * live translation unit, not compiler state: the builder hands one in
	 * while the TU is alive, and this 4-byte mark lets a later walk of the
	 * same TU resolve the declaration again. m_mark follows the definition
	 * when one supersedes a prototype. */
	aclspv_wrd_t	m_mark;

	/* @brief the declared name, as a slice of the cache context's
	 * m_fnlist.m_fn_name. this — not the cursor — is the durable identity:
	 * it is what the duplicate rule compares and what OpName /
	 * LinkageAttributes will need. */
	aclspv_wrd_t	m_idx_name;
	aclspv_wrd_t	m_len_name;

	/* @brief enum ACLSYM_FN_FLAG0_ */
	aclspv_wrd_t	m_flag0;
} aclsym_fn_t;

/**
 * function symbols: the declaration cache pass 1 builds before any body is
 * handed to the emitter.
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
 * a name has exactly one declared function type: a redeclaration whose aclty
 * type value differs is rejected (ACLSPV_COMPILE_MET_INVAL_FN_REDECL). a
 * redeclaration with the same type merges onto the existing symbol, and a
 * definition supersedes an earlier prototype (its cursor becomes the
 * authoritative one).
 *
 * entry points are left out entirely: an entry point is not a call target, so
 * it gets no symbol and its signature is never built. declarations from
 * included headers stay out too: calls into them resolve through clang, and
 * the base header declares overloads this language model does not have.
 * */
aclspv_compile_t aclsym_fn_gather(h_aclspv_ctx_t h_ctx, CXCursor c_root);

/**
 * the symbol for c_fn, matched by cursor first and by spelling as a fallback
 * (clang may hand out a distinct cursor for the same entity).
 *
 * the returned pointer is owned by the slave context's m_fnlist.m_fn and is
 * invalidated by the next symbol append: use it before adding another
 * declaration.
 * */
aclsym_fn_t* aclsym_fn_find(CXCursor c_fn, h_aclspv_ctx_t h_ctx);

/** the symbol emitted as c_id (an OpFunction result id), or 0. */
aclsym_fn_t* aclsym_fn_find_id(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

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
