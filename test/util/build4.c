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
 *
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/spirv.h>

/**
 * aclspv_ctx_build4: the build phase takes no source.
 *
 * two INDEPENDENT scenarios, each on its own context:
 *
 *  1. a translation unit that leaves nothing pending. every body in it has
 *     every callee nameable, so the compile phase emits all of them and the
 *     unit is disposed right after. build4 then closes the module with no
 *     source at all: it decorates the export and requests Linkage. this is the
 *     shape build4 exists for.
 *
 *  2. a translation unit that defers a body (its entry point calls an import).
 *     completing that body means walking the unit, and build4 was given no
 *     source — it must say so instead of walking what may already be gone.
 *
 *  3. a static call cycle. a static function is never imported, so no source
 *     could help either: the failure is the cycle, and build4 must name it.
 *
 *  4. mutual recursion inside one translation unit. both bodies are here, so
 *     the queue could carve them in any order — and that is why the cycle is
 *     rejected explicitly rather than left to the dependency order.
 *
 *  5. the same cycle with ACLSPV_CTX_FLAG0_ALLOW_RECURSION. plain SPIR-V
 *     accepts a recursive call graph; Vulkan does not, so the caller decides.
 *
 *  6. the cycle SPLIT across two translation units. each compile defers its
 *     half; the cache carries the deferred body's cursors so the later source
 *     finishes the earlier one's body.
 *
 *  7. every unit disposed once build returns, then dumped and built again:
 *     nothing may still point into a dead unit.
 * */
static const char* const s_src_lines[] = {
	"int helper(int x) { return x + 1; }\n",
	"__attribute__((visibility(\"default\")))\n",
	"int exported(int x) { return helper(x); }\n",
};
/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

/** a static call cycle: neither body can ever be emitted, because each waits
 * for the other, and a static function can never be imported instead. */
static const char* const s_cycle_lines[] = {
	"static int ping(int x);\n",
	"static int pong(int x) { return ping(x); }\n",
	"static int ping(int x) { return pong(x); }\n",
	"int entry_cycle(int x) { return ping(x); }\n",
};
static char s_cycle[1024];

/** a call cycle inside ONE translation unit: both bodies are here, so the
 * compile phase can finish both — a forward declaration gives the second one
 * an id while the first is being carved. */
static const char* const s_mutual_lines[] = {
	"int g(int x);\n",
	"int f(int x) { return g(x) + 1; }\n",
	"int g(int x) { return f(x) + 2; }\n",
};
static char s_mutual[1024];

static const char* const s_entp_lines[] = {
	"typedef int i32;\n",
	"i32 imp(i32 x);\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_build4(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = imp(1);\n",
	"}\n",
};
static char s_entp[1024];

/** count LinkageAttributes decorations with `c_linkage` kind. */
static unsigned s_count_linkage(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_linkage
		) {
	aclspv_wrdcount_t	i = 5;
	unsigned		n = 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			break;

		/** [op][target][LinkageAttributes][name...][linkage type] */
		if((unsigned)(rd[i] & 0xFFFFu) == (unsigned)SpvOpDecorate
				&& wc >= 4
				&& (unsigned)(rd[i + 2] & 0xFFFFu)
					== (unsigned)SpvDecorationLinkageAttributes
				&& (rd[i + wc - 1] & 0xFFFFu) == c_linkage)
			++n;

		i += wc;
	}

	return n;
}

/** true when the module declares `c_capability`. */
static int s_has_capability(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_capability
		) {
	aclspv_wrdcount_t	i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			break;

		if((unsigned)(rd[i] & 0xFFFFu) == (unsigned)SpvOpCapability
				&& wc >= 2
				&& (rd[i + 1] & 0xFFFFu) == c_capability)
			return 1;

		i += wc;
	}

	return 0;
}

static int s_dump(
		h_aclspv_ctx_t			h_ctx,
		aclspv_wrd_t** ae2f_restrict	wr_spv,
		aclspv_wrdcount_t*		wr_count
		) {
	if(aclspv_dump_ctx_output(h_ctx, wr_count, wr_spv) || !*wr_spv) {
		fputs("[build4] dump failed\n", stderr);
		return 1;
	}

	return 0;
}

/** parse `c_src` into a fresh translation unit and compile it.
 *
 * `c_keep` decides what happens to the translation unit: a source that leaves
 * nothing pending is disposed right there (the shape build4 is for), and a
 * source that does defer is kept, because completing its body means walking
 * it — that is what aclspv_ctx_build1 is for.
 * */
static int s_compile(
		x_aclspv_ctx*		ctx,
		const char*		c_src,
		const aclspv_bool_t	c_keep
		) {
	struct CXUnsavedFile	f;
	CXIndex			idx;
	CXTranslationUnit	tu = 0;
	aclspv_compile_t	ret;

	f.Filename	= "main.c";
	f.Contents	= c_src;
	f.Length	= (unsigned long)strlen(c_src);

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[build4] index failed\n", stderr);
		return 1;
	}

	if(clang_parseTranslationUnit2(
				idx, "main.c"
				, 0, 0
				, &f, 1
				, CXTranslationUnit_None, &tu) != CXError_Success
			|| !tu) {
		fputs("[build4] parse failed\n", stderr);
		clang_disposeIndex(idx);
		return 1;
	}

	ret = aclspv_ctx_compile1(ctx, tu);
	clang_disposeIndex(idx);

	if(c_keep) {
		/** the caller closes the module with this translation unit, then
		 * disposes it: build1 is handed the unit, build4 is handed nothing. */
		if(ret == ACLSPV_COMPILE_OK
				&& aclspv_ctx_build1(ctx, tu) != ACLSPV_COMPILE_OK) {
			fprintf(stderr, "[build4] build1 failed: %d\n"
					, (int)ctx->m_err);
			ret = ctx->m_err;
		}

		clang_disposeTranslationUnit(tu);
	}
	else {
		clang_disposeTranslationUnit(tu);
		tu = 0;
	}

	if(ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4] compile failed: %d\n", (int)ret);
		return 1;
	}

	return 0;
}

/** scenario 1: the TU defers nothing, is disposed, and build4 closes.
 *
 * expected module: helper + exported (2 bodies), one call exported->helper,
 * one Export decoration and no Import, under the Linkage capability.
 * */
static int s_case_no_pending(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	count = 0;
	unsigned		c_fn;
	unsigned		c_call;
	acltest_spirv_stats	stats;
	int			ret = EXIT_FAILURE;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/1] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	/** the unit leaves nothing pending, so it is disposed right after the
	 * compile phase and build4 closes the module with no source. */
	if(s_compile(&ctx, s_src, 0)) {
		fputs("[build4/1] compile failed\n", stderr);
		goto LBL_DONE;
	}

	if(aclspv_ctx_build4(&ctx) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/1] build4 failed: %d\n", (int)ctx.m_err);
		goto LBL_DONE;
	}

	/** a second build4 has nothing left to say: every decoration is on the
	 * symbol already, so it must not decorate twice. */
	if(aclspv_ctx_build4(&ctx) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/1] second build4 failed: %d\n"
				, (int)ctx.m_err);
		goto LBL_DONE;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_DONE;

	if(acltest_spirv_scan(spv, count, &stats) != 0) {
		fputs("[build4/1] malformed spir-v instruction stream\n", stderr);
		goto LBL_DONE;
	}

	c_fn	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunctionCall);

	printf("[build4/1] fn=%u call=%u export=%u import=%u linkcap=%d\n"
			, c_fn, c_call
			, s_count_linkage(spv, count, (aclspv_wrd_t)SpvLinkageTypeExport)
			, s_count_linkage(spv, count, (aclspv_wrd_t)SpvLinkageTypeImport)
			, s_has_capability(
				spv, count, (aclspv_wrd_t)SpvCapabilityLinkage));

	/** helper and exported are the two bodies; exported->helper is the one
	 * call. the export decoration is what build4 added — once, even though it
	 * ran twice — and Linkage is what makes it legal. nothing is imported. */
	if(c_fn != 2 || c_call != 1
			|| s_count_linkage(
				spv, count, (aclspv_wrd_t)SpvLinkageTypeExport) != 1
			|| s_count_linkage(
				spv, count, (aclspv_wrd_t)SpvLinkageTypeImport) != 0
			|| !s_has_capability(
				spv, count, (aclspv_wrd_t)SpvCapabilityLinkage)) {
		fputs("[build4/1] build4 did not close the module\n", stderr);
		goto LBL_DONE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[build4/1] cannot write %s\n", ACLTEST_OUT);
			goto LBL_DONE;
		}

		fwrite(spv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	ret = EXIT_SUCCESS;

LBL_DONE:
	free(spv);
	aclspv_fini_ctx(&ctx);
	return ret;
}

/** scenario 2: the TU defers a body, so build4 must refuse to close it.
 * */
static int s_case_pending(void) {
	x_aclspv_ctx	ctx;
	aclspv_usrcfg_t	cfg;
	int		ret = EXIT_SUCCESS;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/2] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	/** the entry point calls an import, so its body is deferred. */
	if(s_compile(&ctx, s_entp, 0)) {
		fputs("[build4/2] compile failed\n", stderr);
		ret = EXIT_FAILURE;
		goto LBL_DONE;
	}

	/** no source was given, and completing the body would mean walking the
	 * translation unit that is already gone: build4 must report it. */
	if(aclspv_ctx_build4(&ctx) == ACLSPV_COMPILE_OK) {
		fputs("[build4/2] build4 closed a module with a deferred body\n"
				, stderr);
		ret = EXIT_FAILURE;
		goto LBL_DONE;
	}

	printf("[build4/2] build4 refused a deferred body: %d (want %d)\n"
			, (int)ctx.m_err, (int)ACLSPV_COMPILE_HOST_PENDING);

	if(ctx.m_err != ACLSPV_COMPILE_HOST_PENDING) {
		fputs("[build4/2] build4 did not name the reason\n", stderr);
		ret = EXIT_FAILURE;
	}

LBL_DONE:
	aclspv_fini_ctx(&ctx);
	return ret;
}

/** scenario 3: a static call cycle.
 *
 * a static function is never imported, so its body can only come out of its own
 * translation unit. a cycle among them cannot be expressed either: SPIR-V has
 * no recursive OpFunctionCall. the compile phase is where that is found, since
 * it is the phase that sees both bodies.
 * */
static int s_case_static_cycle(void) {
	x_aclspv_ctx	ctx;
	aclspv_usrcfg_t	cfg;
	int		ret = EXIT_SUCCESS;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/3] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	if(s_compile(&ctx, s_cycle, 0)) {
		printf("[build4/3] compile named a static cycle: %d (want %d)\n"
				, (int)ctx.m_err, (int)ACLSPV_COMPILE_RECURSIVE_CALL);

		if(ctx.m_err != ACLSPV_COMPILE_RECURSIVE_CALL) {
			fputs("[build4/3] a static cycle was reported as something else\n"
					, stderr);
			ret = EXIT_FAILURE;
		}

		goto LBL_DONE;
	}

	fputs("[build4/3] a static call cycle compiled\n", stderr);
	ret = EXIT_FAILURE;

LBL_DONE:
	aclspv_fini_ctx(&ctx);
	return ret;
}

/** scenario 4: mutual recursion inside one translation unit.
 *
 * `f` calls `g` before `g` is defined, and `g` calls `f` back. both bodies are
 * here, so the queue can carve them in any order — and that is exactly why the
 * cycle has to be rejected explicitly: the module would otherwise carry a
 * recursive OpFunctionCall, which SPIR-V does not have.
 * */
static int s_case_mutual_one_tu(void) {
	x_aclspv_ctx	ctx;
	aclspv_usrcfg_t	cfg;
	int		ret = EXIT_SUCCESS;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/4] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	if(s_compile(&ctx, s_mutual, 0)) {
		printf("[build4/4] compile named a mutual recursion: %d (want %d)\n"
				, (int)ctx.m_err, (int)ACLSPV_COMPILE_RECURSIVE_CALL);

		if(ctx.m_err != ACLSPV_COMPILE_RECURSIVE_CALL) {
			fputs("[build4/4] mutual recursion was reported as something "
					"else\n", stderr);
			ret = EXIT_FAILURE;
		}

		goto LBL_DONE;
	}

	fputs("[build4/4] a mutual recursion compiled\n", stderr);
	ret = EXIT_FAILURE;

LBL_DONE:
	aclspv_fini_ctx(&ctx);
	return ret;
}

/** scenario 5: ALLOW_RECURSION keeps the cycle.
 *
 * the flag is the caller's statement that the target tolerates recursion —
 * Vulkan does not ([VUID-StandaloneSpirv-None-04634]), plain SPIR-V does. with
 * it on, a cycle is emitted instead of rejected: the ids are reserved up front
 * so every body of the cycle can be carved, and no placeholder OpFunction is
 * left behind.
 * */
static int s_case_recursion_allowed(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	struct CXUnsavedFile	f;
	CXIndex			idx;
	CXTranslationUnit	tu = 0;
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	count = 0;
	unsigned		c_fn;
	unsigned		c_call;
	acltest_spirv_stats	stats;
	int			ret = EXIT_SUCCESS;

	memset(&cfg, 0, sizeof(cfg));
	cfg.m_flag0 = ACLSPV_CTX_FLAG0_ALLOW_RECURSION;
	cfg.m_ver_spv = aclspv_ver_spv(1, 3);

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/5] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	f.Filename	= "main.c";
	f.Contents	= s_mutual;
	f.Length	= (unsigned long)strlen(s_mutual);

	idx = clang_createIndex(0, 0);

	if(!idx
			|| clang_parseTranslationUnit2(
				idx, "main.c", 0, 0, &f, 1
				, CXTranslationUnit_None, &tu) != CXError_Success
			|| !tu) {
		fputs("[build4/5] parse failed\n", stderr);
		clang_disposeIndex(idx);
		aclspv_fini_ctx(&ctx);
		return EXIT_FAILURE;
	}

	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_build4(&ctx) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/5] allowed recursion failed: %d\n"
				, (int)ctx.m_err);
		ret = EXIT_FAILURE;
		goto LBL_DONE;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_DONE;

	if(acltest_spirv_scan(spv, count, &stats) != 0) {
		fputs("[build4/5] malformed spir-v instruction stream\n", stderr);
		ret = EXIT_FAILURE;
		goto LBL_DONE;
	}

	c_fn	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunctionCall);

	printf("[build4/5] recursion allowed: fn=%u call=%u\n", c_fn, c_call);

	/** f and g, each defined exactly once — an id is reserved for the cycle,
	 * never declared twice — and the two calls that make the cycle. */
	if(c_fn != 2 || c_call != 2) {
		fputs("[build4/5] allowed recursion did not emit the cycle\n"
				, stderr);
		ret = EXIT_FAILURE;
	}

LBL_DONE:
	free(spv);
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(idx);
	aclspv_fini_ctx(&ctx);
	return ret;
}

/** scenario 6: mutual recursion ACROSS translation units.
 *
 * a.c defines `f`, which calls `g`; b.c defines `g`, which calls `f`. neither
 * compile pass can see the other body, so each defers — and the symbol cache
 * keeps the deferred body's cursors for the next pass to pick up
 * (s_collect_deferred). by the time b.c is compiled, `f`'s callee is out.
 *
 * that is why compile1 documents "the TU must outlive the build": the cache
 * holds cursors into both units. this test keeps them, which is the contract.
 * */
static int s_case_recursion_across_tu(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	struct CXUnsavedFile	f[2];
	const char*		ARG[] = {"m.c"};
	CXIndex			idx;
	CXTranslationUnit	tu[2];
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	count = 0;
	unsigned		c_fn;
	unsigned		c_call;
	acltest_spirv_stats	stats;
	int			ret = EXIT_FAILURE;
	int			k;

	memset(&cfg, 0, sizeof(cfg));
	cfg.m_flag0 = ACLSPV_CTX_FLAG0_ALLOW_RECURSION;
	cfg.m_ver_spv = aclspv_ver_spv(1, 3);

	tu[0] = 0;
	tu[1] = 0;

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/6] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[build4/6] index failed\n", stderr);
		aclspv_fini_ctx(&ctx);
		return EXIT_FAILURE;
	}

	f[0].Filename	= "a.c";
	f[0].Contents	= "int g(int x);\nint f(int x) { return g(x) + 1; }\n";
	f[0].Length	= (unsigned long)strlen(f[0].Contents);

	if(clang_parseTranslationUnit2(
				idx, "a.c", ARG, 0, f, 1
				, CXTranslationUnit_None, &tu[0]) != CXError_Success
			|| !tu[0]
			|| aclspv_ctx_compile1(&ctx, tu[0]) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/6] a.c failed: %d\n", (int)ctx.m_err);
		goto LBL_DONE;
	}

	f[0].Filename	= "b.c";
	f[0].Contents	= "int f(int x);\nint g(int x) { return f(x) + 2; }\n";
	f[0].Length	= (unsigned long)strlen(f[0].Contents);

	if(clang_parseTranslationUnit2(
				idx, "b.c", ARG, 0, f, 1
				, CXTranslationUnit_None, &tu[1]) != CXError_Success
			|| !tu[1]
			|| aclspv_ctx_compile1(&ctx, tu[1]) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/6] b.c failed: %d\n", (int)ctx.m_err);
		goto LBL_DONE;
	}

	if(aclspv_ctx_build4(&ctx) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/6] build4 failed: %d\n", (int)ctx.m_err);
		goto LBL_DONE;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_DONE;

	if(acltest_spirv_scan(spv, count, &stats) != 0) {
		fputs("[build4/6] malformed spir-v instruction stream\n", stderr);
		goto LBL_DONE;
	}

	c_fn	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunctionCall);

	printf("[build4/6] recursion across TU: fn=%u call=%u\n", c_fn, c_call);

	/** f and g, each defined once, and the two calls that make the cycle:
	 * the module a single-source cycle would have produced. */
	if(c_fn != 2 || c_call != 2) {
		fputs("[build4/6] mutual recursion across units did not close\n"
				, stderr);
		goto LBL_DONE;
	}

	ret = EXIT_SUCCESS;

LBL_DONE:
	free(spv);
	for(k = 0; k < 2; ++k)
		clang_disposeTranslationUnit(tu[k]);
	clang_disposeIndex(idx);
	aclspv_fini_ctx(&ctx);
	return ret;
}

/** scenario 7: every translation unit is disposable once build returns.
 *
 * build is the last phase, so the cache must not still point into any unit.
 * this compiles two sources, builds, disposes both units, and then dumps and
 * builds AGAIN: a cursor left behind would show up as a crash or a second
 * decoration here, and as a failure in spirv-val.
 * */
static int s_case_disposable_after_build(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	struct CXUnsavedFile	f[2];
	const char*		ARG[] = {"m.c"};
	CXIndex			idx;
	CXTranslationUnit	tu[2];
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	count = 0;
	unsigned		c_fn;
	unsigned		c_call;
	acltest_spirv_stats	stats;
	int			ret = EXIT_FAILURE;
	int			k;

	memset(&cfg, 0, sizeof(cfg));
	cfg.m_flag0 = ACLSPV_CTX_FLAG0_ALLOW_RECURSION;
	cfg.m_ver_spv = aclspv_ver_spv(1, 3);

	tu[0] = 0;
	tu[1] = 0;

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[build4/7] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[build4/7] index failed\n", stderr);
		aclspv_fini_ctx(&ctx);
		return EXIT_FAILURE;
	}

	f[0].Filename	= "a.c";
	f[0].Contents	= "int g(int x);\nint f(int x) { return g(x) + 1; }\n";
	f[0].Length	= (unsigned long)strlen(f[0].Contents);

	f[1].Filename	= "b.c";
	f[1].Contents	= "int f(int x);\nint g(int x) { return f(x) + 2; }\n";
	f[1].Length	= (unsigned long)strlen(f[1].Contents);

	if(clang_parseTranslationUnit2(
				idx, "a.c", ARG, 0, &f[0], 1
				, CXTranslationUnit_None, &tu[0]) != CXError_Success
			|| !tu[0]
			|| clang_parseTranslationUnit2(
				idx, "b.c", ARG, 0, &f[1], 1
				, CXTranslationUnit_None, &tu[1]) != CXError_Success
			|| !tu[1]) {
		fputs("[build4/7] parse failed\n", stderr);
		goto LBL_DONE;
	}

	if(aclspv_ctx_compile1(&ctx, tu[0]) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_compile1(&ctx, tu[1]) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_build4(&ctx) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/7] compile/build failed: %d\n"
				, (int)ctx.m_err);
		goto LBL_DONE;
	}

	/** build is over: the units are the caller's to drop, and dropping them
	 * must leave the context fully usable. */
	clang_disposeTranslationUnit(tu[0]);
	clang_disposeTranslationUnit(tu[1]);
	tu[0] = 0;
	tu[1] = 0;

	if(s_dump(&ctx, &spv, &count))
		goto LBL_DONE;

	if(acltest_spirv_scan(spv, count, &stats) != 0) {
		fputs("[build4/7] malformed spir-v instruction stream\n", stderr);
		goto LBL_DONE;
	}

	c_fn	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunctionCall);

	/** a second build has nothing left to say, and must not reach for a unit
	 * that is gone. */
	if(aclspv_ctx_build4(&ctx) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[build4/7] second build4 failed: %d\n"
				, (int)ctx.m_err);
		goto LBL_DONE;
	}

	printf("[build4/7] disposable after build: fn=%u call=%u\n"
			, c_fn, c_call);

	if(c_fn != 2 || c_call != 2) {
		fputs("[build4/7] the module changed after the units were "
				"disposed\n", stderr);
		goto LBL_DONE;
	}

	ret = EXIT_SUCCESS;

LBL_DONE:
	free(spv);
	for(k = 0; k < 2; ++k)
		clang_disposeTranslationUnit(tu[k]);
	clang_disposeIndex(idx);
	aclspv_fini_ctx(&ctx);
	return ret;
}

int main(void) {
	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));
	acltest_concat(s_entp, s_entp_lines,
			(unsigned)(sizeof(s_entp_lines) / sizeof(s_entp_lines[0])));

	acltest_concat(s_cycle, s_cycle_lines,
			(unsigned)(sizeof(s_cycle_lines) / sizeof(s_cycle_lines[0])));

	if(s_case_no_pending())
		return EXIT_FAILURE;

	if(s_case_pending())
		return EXIT_FAILURE;

	acltest_concat(s_mutual, s_mutual_lines,
			(unsigned)(sizeof(s_mutual_lines) / sizeof(s_mutual_lines[0])));

	if(s_case_static_cycle())
		return EXIT_FAILURE;

	if(s_case_mutual_one_tu())
		return EXIT_FAILURE;

	if(s_case_recursion_allowed())
		return EXIT_FAILURE;

	if(s_case_recursion_across_tu())
		return EXIT_FAILURE;

	return s_case_disposable_after_build();
}
