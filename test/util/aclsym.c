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

#include <aclspv.h>

#include "../../lib/aclsym/fn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	CHECK(c)	do { \
	if(!(c)) { \
		fprintf(stderr, "check failed: %s:%d\n", __FILE__, __LINE__); \
		return 1; \
	} \
} while(0)

static int s_compile(
		struct CXUnsavedFile*	rdwr_files,
		unsigned		c_files,
		aclspv_compile_t*	wr_ret,
		x_aclspv_ctx*		wr_ctx
		) {
	const char*	ARG[] = {"main.c"};
	aclspv_usrcfg_t	cfg;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(wr_ctx, &cfg)) {
		fputs("[aclsym] init failed\n", stderr);
		return 1;
	}

	*wr_ret = aclspv_ctx_build3(wr_ctx, rdwr_files, c_files, ARG, 1, 0);

	return 0;
}


/** count the emitted instructions of interest. the module starts with the
 * 5-word SPIR-V header; every instruction is (wordcount << 16) | opcode. */
static int s_scan(
		x_aclspv_ctx*	ctx,
		unsigned*	wr_fn,
		unsigned*	wr_tyfn
		) {
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	c_n = 0;
	aclspv_wrdcount_t	i = 5;

	*wr_fn = 0;
	*wr_tyfn = 0;

	if(aclspv_dump_ctx_output(ctx, &c_n, &spv) || !spv) {
		fputs("[aclsym] dump failed\n", stderr);
		return 1;
	}

	while(i < c_n) {
		const aclspv_wrd_t	w = spv[i];
		const unsigned		op = (unsigned)(w & 0xFFFFu);
		const unsigned		wc = (unsigned)(w >> 16);

		if(op == (unsigned)SpvOpFunction)	++*wr_fn;
		if(op == (unsigned)SpvOpTypeFunction)	++*wr_tyfn;

		i += wc ? wc : (c_n - i);
	}

	free(spv);
	return 0;
}

/** a declaration cached, a redeclaration merged, a definition superseding the
 * prototype, equal signatures sharing one type id, a declaration from an
 * include excluded, and an entry point left out entirely.
 *
 * the symbols are asserted in declaration order: the cursors they hold die
 * with the translation unit, so the test must not call clang after the
 * compile returns. */
static int s_case_ok(void) {
	static const char src[] =
		"#include \"aux.h\"\n"
		"int helper(int a);\n"
		"int helper(int a);\n"
		"int helper(int a) { return a; }\n"
		"int twin(int b);\n"
		"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
		"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
		"void t_aclsym(void) { }\n";

	static const char aux[] = "int hdr_helper(int);\n";

	x_aclspv_ctx		ctx;
	aclspv_compile_t	ret;
	struct CXUnsavedFile	files[2];
	aclsym_fn_t*		helper;
	aclsym_fn_t*		twin;

	files[0].Contents	= src;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(src) - 1;

	files[1].Contents	= aux;
	files[1].Filename	= "aux.h";
	files[1].Length		= sizeof(aux) - 1;

	if(s_compile(files, 2, &ret, &ctx))
		return 1;

	if(ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[aclsym] ok-case compile failed: %d\n", (int)ret);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	/** the cache lives in the scratch context: pass 1 builds real aclty
	 * types, and the emitting module must not see them. */
	if(!ctx.m_ctx_slave) {
		fputs("[aclsym] no scratch context\n", stderr);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	/** helper, twin, and t_aclsym. hdr_helper came from aux.h; an entry
	 * point is not a call target, but it is still recorded (ENTRY) so a build
	 * phase that takes no source can tell whether its body has gone out. */
	if(ctx.m_ctx_slave->m_fnlist.m_num_fn != 3) {
		fprintf(stderr, "[aclsym] symbol count: %u\n"
				, (unsigned)ctx.m_ctx_slave->m_fnlist.m_num_fn);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	helper = &((aclsym_fn_t*)ctx.m_ctx_slave->m_fnlist.m_fn.m_p)[0];
	twin = &((aclsym_fn_t*)ctx.m_ctx_slave->m_fnlist.m_fn.m_p)[1];

	/** three `helper` declarations, one symbol; the definition won and its
	 * body was emitted. */
	CHECK(helper->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED));
	CHECK(helper->m_type_id);
	CHECK(helper->m_id);
	CHECK(helper->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED));
	/** `return a;` reads its own parameter and writes nothing. */
	CHECK(helper->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST));
	CHECK(helper->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE));

	/** a different function with the same signature interns to the same
	 * type id: this is the value comparison the duplicate check relies on. */
	CHECK(twin->m_type_id == helper->m_type_id);
	CHECK(!(twin->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED)));

	/** the entry point's record: marked ENTRY, never NAMED, so no call
	 * resolves to it and no export decorates it. */
	{
		const aclsym_fn_t* const ENTP
			= &((aclsym_fn_t*)ctx.m_ctx_slave->m_fnlist.m_fn.m_p)[2];

		CHECK(ENTP->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_ENTRY));
		CHECK(!(ENTP->m_flag0 & ACLSYM_FN_FLAG0_NAMED));
	}

	/** the gather pass is a cache, not an emitter: of the three functions
	 * in this file only the entry point has a body, so exactly one
	 * OpFunction must reach the module. no OpFunction is emitted for a
	 * cached declaration. */
	{
		unsigned c_fn = 0;
		unsigned c_tyfn = 0;

		if(s_scan(&ctx, &c_fn, &c_tyfn)) {
			aclspv_fini_ctx(&ctx);
			return 1;
		}

		/** helper and the entry point. two signatures: int(int) and void(). */
		if(c_fn != 2 || c_tyfn != 2) {
			fprintf(stderr, "[aclsym] emitted: %u function, %u function type\n"
					, c_fn, c_tyfn);
			aclspv_fini_ctx(&ctx);
			return 1;
		}

		printf("[aclsym] cached: OpFunction=%u OpTypeFunction=%u\n"
				, c_fn, c_tyfn);
	}

	aclspv_fini_ctx(&ctx);
	return 0;
}

/** an entry point is never a call target: no signature, no import, no export.
 * it is recorded as an ENTRY symbol so a build without a source can still
 * tell that its body went out, and its body is emitted as the root of the
 * module. */
static int s_case_entry_only(void) {
	static const char src[] =
		"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
		"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
		"void t_entry_only(void) { }\n";

	x_aclspv_ctx		ctx;
	aclspv_compile_t	ret;
	struct CXUnsavedFile	files[1];
	unsigned		c_fn = 0;
	unsigned		c_tyfn = 0;

	files[0].Contents	= src;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(src) - 1;

	if(s_compile(files, 1, &ret, &ctx))
		return 1;

	if(ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[aclsym] entry-only compile failed: %d\n", (int)ret);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	if(!ctx.m_ctx_slave
			|| ctx.m_ctx_slave->m_fnlist.m_num_fn != 1
			|| ctx.m_fnlist.m_num_entp != 1) {
		fprintf(stderr, "[aclsym] entry-only: %u symbol, %u entry\n"
				, (unsigned)(ctx.m_ctx_slave
					? ctx.m_ctx_slave->m_fnlist.m_num_fn : 0)
				, (unsigned)ctx.m_fnlist.m_num_entp);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	if(s_scan(&ctx, &c_fn, &c_tyfn)) {
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	printf("[aclsym] entry-only: OpFunction=%u OpTypeFunction=%u\n"
			, c_fn, c_tyfn);

	if(c_fn != 1) {
		fprintf(stderr, "[aclsym] entry-only functions: %u, expected 1\n", c_fn);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	aclspv_fini_ctx(&ctx);
	return 0;
}

/** the same name twice with different canonical types: rejected before any
 * body is parsed. */
static int s_case_conflict(void) {
	static const char src[] =
		"int dup(int a);\n"
		"int dup(float a);\n";

	x_aclspv_ctx		ctx;
	aclspv_compile_t	ret;
	struct CXUnsavedFile	files[1];

	files[0].Contents	= src;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(src) - 1;

	if(s_compile(files, 1, &ret, &ctx))
		return 1;

	if(ret != ACLSPV_COMPILE_MET_INVAL_FN_REDECL) {
		fprintf(stderr, "[aclsym] conflict: got %d, expected %d\n"
				, (int)ret
				, (int)ACLSPV_COMPILE_MET_INVAL_FN_REDECL);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	aclspv_fini_ctx(&ctx);
	return 0;
}


/** the FunctionDecl named c_name in the live TU. */
struct s_pick {
	const char*	m_name;
	CXCursor	m_cur;
};

static enum CXChildVisitResult s_pick_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_client
		) {
	struct s_pick* ae2f_restrict p = rdwr_client;
	CXString	S;
	const char*	s;
	int		hit;

	if(c_cur.kind != CXCursor_FunctionDecl)
		return CXChildVisit_Continue;

	S = clang_getCursorSpelling(c_cur);
	s = clang_getCString(S);
	hit = (int)(s && !strcmp(s, p->m_name));
	clang_disposeString(S);

	if(hit) {
		p->m_cur = c_cur;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}

static CXCursor s_decl(CXCursor c_root, const char* c_name) {
	struct s_pick p;

	p.m_name	= c_name;
	p.m_cur		= clang_getNullCursor();

	clang_visitChildren(c_root, s_pick_visit, &p);

	return p.m_cur;
}

/** dependencies: body readiness, import detection, and the declared purity of
 * a body-less callee. no symbol keeps a cursor: the test holds the live TU and
 * hands the cursor in, exactly as the compiler does. */
static int s_case_ready(void) {
	static const char src[] =
		"int leaf(int x) { return x + 1; }\n"
		"int mid(int x) { return leaf(x); }\n"
		"int self(int x) { return x ? self(x - 1) : 0; }\n"
		"int ext(int x);\n"
		"int uses_ext(int x) { return ext(x); }\n"
		"int uses_builtin(int x) { return __builtin_abs(x); }\n"
		"__attribute__((pure)) int pure_ext(int x);\n"
		"__attribute__((const)) int const_ext(int x);\n"
		"int plain_ext(int x);\n";

	CXIndex			idx;
	CXTranslationUnit	tu = 0;
	CXCursor		root;
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	struct CXUnsavedFile	files[1];

	CXCursor		c_leaf, c_mid, c_self, c_ext, c_uses_ext, c_uses_builtin;
	CXCursor		c_pure_ext, c_const_ext, c_plain_ext;

	aclsym_fn_t*		leaf;
	aclsym_fn_t*		mid;
	aclsym_fn_t*		ext;
	aclsym_fn_t*		self;
	aclsym_fn_t*		uses_ext;
	aclsym_fn_t*		uses_builtin;
	aclsym_fn_t*		pure_ext;
	aclsym_fn_t*		const_ext;
	aclsym_fn_t*		plain_ext;

	memset(&cfg, 0, sizeof(cfg));
	memset(files, 0, sizeof(files));

	files[0].Filename	= "main.c";
	files[0].Contents	= src;
	files[0].Length		= sizeof(src) - 1;

	idx = clang_createIndex(0, 0);
	CHECK(idx);

	CHECK(clang_parseTranslationUnit2(
				idx, "main.c"
				, 0, 0
				, files, 1
				, CXTranslationUnit_None, &tu) == CXError_Success);
	CHECK(tu);

	root = clang_getTranslationUnitCursor(tu);

	CHECK(!aclspv_init_ctx(&ctx, &cfg));
	CHECK(aclsym_fn_gather(&ctx, root) == ACLSPV_COMPILE_OK);

	c_leaf		= s_decl(root, "leaf");
	c_mid		= s_decl(root, "mid");
	c_self		= s_decl(root, "self");
	c_ext		= s_decl(root, "ext");
	c_uses_ext	= s_decl(root, "uses_ext");
	c_uses_builtin	= s_decl(root, "uses_builtin");
	c_pure_ext	= s_decl(root, "pure_ext");
	c_const_ext	= s_decl(root, "const_ext");
	c_plain_ext	= s_decl(root, "plain_ext");

	CHECK(!clang_Cursor_isNull(c_leaf) && !clang_Cursor_isNull(c_mid));
	CHECK(!clang_Cursor_isNull(c_self) && !clang_Cursor_isNull(c_uses_ext));
	CHECK(!clang_Cursor_isNull(c_uses_builtin));

	leaf		= aclsym_fn_find(c_leaf, &ctx);
	mid		= aclsym_fn_find(c_mid, &ctx);
	ext		= aclsym_fn_find(c_ext, &ctx);
	self		= aclsym_fn_find(c_self, &ctx);
	uses_ext	= aclsym_fn_find(c_uses_ext, &ctx);
	uses_builtin	= aclsym_fn_find(c_uses_builtin, &ctx);
	pure_ext	= aclsym_fn_find(c_pure_ext, &ctx);
	const_ext	= aclsym_fn_find(c_const_ext, &ctx);
	plain_ext	= aclsym_fn_find(c_plain_ext, &ctx);

	CHECK(leaf && mid && self && ext && uses_ext && uses_builtin);
	CHECK(pure_ext && const_ext && plain_ext);

	/** leaf calls nothing. */
	CHECK(aclsym_fn_ready(c_leaf, &ctx));

	/** mid calls leaf, whose body is not emitted yet. */
	CHECK(!aclsym_fn_ready(c_mid, &ctx));

	/** a self call is a cycle: recursion has no SPIR-V form, so the body
	 * never becomes emittable. */
	CHECK(!aclsym_fn_ready(c_self, &ctx));

	/** ext has no body here: nothing will be emitted for it, so the caller
	 * waits for the build phase to declare the import. */
	CHECK(!aclsym_fn_ready(c_uses_ext, &ctx));

	/** not a cached symbol at all: a builtin, not a function call. */
	CHECK(aclsym_fn_ready(c_uses_builtin, &ctx));

	/** once the callee is out, the caller is ready. */
	leaf->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED);
	CHECK(aclsym_fn_ready(c_mid, &ctx));

	/** the import declaration gives ext an id; its caller is now ready.
	 * the import itself is still what aclsym_fn_needs_import reports. */
	ext->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED);
	CHECK(aclsym_fn_ready(c_uses_ext, &ctx));

	/** the compile phase defers exactly the callers of body-less normal
	 * functions; a defined callee is an ordering problem, not an import. */
	CHECK(aclsym_fn_needs_import(c_uses_ext, &ctx));
	CHECK(!aclsym_fn_needs_import(c_mid, &ctx));
	CHECK(!aclsym_fn_needs_import(c_self, &ctx));
	CHECK(!aclsym_fn_needs_import(c_uses_builtin, &ctx));
	CHECK(!aclsym_fn_needs_import(c_leaf, &ctx));

	/** the declared promise of a callee with no body: const implies pure. */
	CHECK(pure_ext->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE));
	CHECK(!(pure_ext->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST)));

	CHECK(const_ext->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE));
	CHECK(const_ext->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST));

	CHECK(!(plain_ext->m_flag0 & (
					aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE)
					| aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST))));

	aclspv_fini_ctx(&ctx);
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(idx);

	return 0;
}

/** internal-linkage functions are cached apart from the normal symbols and
 * keep a plain C name. */
static int s_case_static(void) {
	static const char src[] =
		"static int hidden(int a) { return a + 1; }\n"
		"int uses_hidden(int a) { return hidden(a); }\n"
		"int visible(int a);\n";

	x_aclspv_ctx		ctx;
	aclspv_compile_t	ret;
	struct CXUnsavedFile	files[1];
	aclsym_fn_t*		hidden;
	const char*		name;

	files[0].Contents	= src;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(src) - 1;

	if(s_compile(files, 1, &ret, &ctx))
		return 1;

	if(ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[aclsym] static-case compile failed: %d\n", (int)ret);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	if(!ctx.m_ctx_slave
			|| ctx.m_ctx_slave->m_fnlist.m_num_fn != 2
			|| ctx.m_ctx_slave->m_fnlist.m_num_fn_static != 1) {
		fprintf(stderr, "[aclsym] static: %u normal, %u static\n"
				, (unsigned)(ctx.m_ctx_slave
					? ctx.m_ctx_slave->m_fnlist.m_num_fn : 0)
				, (unsigned)(ctx.m_ctx_slave
					? ctx.m_ctx_slave->m_fnlist.m_num_fn_static : 0));
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	hidden = &((aclsym_fn_t*)ctx.m_ctx_slave->m_fnlist.m_fn_static.m_p)[0];
	name = aclsym_fn_name(hidden, &ctx);

	CHECK(hidden->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_STATIC));
	CHECK(hidden->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED));
	CHECK(hidden->m_len_usr != 0);
	CHECK(name && !strcmp(name, "hidden"));

	/** static names live in their own arena: the normal name arena holds
	 * only uses_hidden and visible (11 + 1 + 7 + 1 bytes). */
	CHECK(ctx.m_ctx_slave->m_fnlist.m_num_fn_name == 20);
	CHECK(ctx.m_ctx_slave->m_fnlist.m_num_fn_static_name != 0);

	printf("[aclsym] static: count=%u name=%s\n"
			, (unsigned)ctx.m_ctx_slave->m_fnlist.m_num_fn_static
			, name ? name : "");

	aclsym_fn_reset_static(&ctx);
	CHECK(ctx.m_ctx_slave->m_fnlist.m_num_fn_static == 0);
	CHECK(ctx.m_ctx_slave->m_fnlist.m_num_fn_static_name == 0);

	aclspv_fini_ctx(&ctx);
	return 0;
}

int main(void) {
	if(s_case_ok())
		return 1;

	if(s_case_entry_only())
		return 1;

	if(s_case_conflict())
		return 1;

	if(s_case_ready())
		return 1;

	if(s_case_static())
		return 1;

	return 0;
}
