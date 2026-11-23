#include "./bodies.h"

#include "./fndecl/imports.h"
#include "./fndecl/norm.h"
#include "./fndecl/entp.h"
#include "./root.h"

#include <aclsym/fn.h>

#include <aclcur/common.h>

#include <aclmem.h>
#include <aclutil/wrdemit.h>
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

struct s_collect {
	x_aclspv_vec*		m_buf;
	h_aclspv_ctx_t		m_ctx;
	aclspv_wrd_t		m_num;

	/** building: only what the compile phase left behind. compiling: every
	 * body with no OpFunction yet, including the entry points it may still
	 * manage to emit. */
	aclspv_bool_t		m_build;
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
			|| c_par.kind != CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	unless(clang_Location_isFromMainFile(clang_getCursorLocation(c_par)))
		return CXChildVisit_Continue;

	p_sym = aclsym_fn_find(c_par, p->m_ctx);

	if(p_sym) {
		/** a defined function: the body is out already, or this phase will
		 * schedule it. */
		if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))
			return CXChildVisit_Continue;

		ITEM.m_entry = 0;
	}
	else {
		/** no symbol: an entry point. in the build phase a ready body was
		 * already emitted by the compile phase, so it is skipped; in the
		 * compile phase every entry body is collected and the queue decides
		 * when it goes out. */
		if(p->m_build && aclsym_body_ready(c_cur, p->m_ctx))
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
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
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
	return c_it->m_entry
		? aclsym_body_ready(c_it->m_body, h_ctx)
		: aclsym_fn_ready(c_it->m_fn, h_ctx);
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
	col.m_build	= c_build;

	clang_visitChildren(c_root, s_collect_visit, &col);

	if(h_ctx->m_err) {
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
				h_ctx->m_err = ACLSPV_COMPILE_RECURSIVE_CALL;
				break;
			}
		}
	}

	aclmem_fini(&buf);

	return h_ctx->m_err;
}

aclspv_compile_t aclemit_compile_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	return s_run(h_ctx, c_root, 0);
}

aclspv_compile_t aclemit_build_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	return s_run(h_ctx, c_root, 1);
}
