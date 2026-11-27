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

#include <acltest/onefile.h>
#include <acltest/spirv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * definition order must not matter: the compile phase emits bodies in
 * dependency order (aclemit/bodies.c), so a body that calls a function
 * defined further down is emitted after the callee, still in the compile
 * phase. the build phase then has nothing left to do.
 *
 * a call cycle can never be completed — recursion has no representation in
 * the target — so the build phase reports it instead of hanging or crashing.
 * */

static const char* const s_fwd_lines[] = {
	"typedef int i32;\n",
	"i32 later(i32 x);\n",
	"i32 caller(i32 x) { return later(x); }\n",
	"i32 later(i32 x) { return x + 1; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_fwd(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = caller(1);\n",
	"}\n",
};

static const char* const s_rec_lines[] = {
	"typedef int i32;\n",
	"i32 rec(i32 x) { if(x) return rec(x - 1); return 0; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_rec(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = rec(1);\n",
	"}\n",
};

/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

/** counts of the instructions this test cares about. */
struct s_counts {
	unsigned	m_fn;
	unsigned	m_call;
	unsigned	m_entp;
};

static int s_counts(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		struct s_counts*	wr
		) {
	acltest_spirv_stats stats;

	if(acltest_spirv_scan(rd, c, &stats) != 0) {
		fputs("[forward] malformed spir-v instruction stream\n", stderr);
		return 1;
	}

	wr->m_fn	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpFunction);
	wr->m_call	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpFunctionCall);
	wr->m_entp	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpEntryPoint);

	return 0;
}

/** parse `rd_lines` into a caller-owned TU (the compile | build contract). */
static int s_parse(
		const char* const*	rd_lines,
		unsigned		c_num,
		CXIndex			h_idx,
		CXTranslationUnit*	wr_tu
		) {
	struct CXUnsavedFile	files[2];

	acltest_concat(s_src, rd_lines, c_num);

	files[0].Contents	= s_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(s_src);

	files[1].Contents	= s_oclbase;
	files[1].Filename	= "opencl-c-base.h";
	files[1].Length		= sizeof(s_oclbase) - 1;

	if(clang_parseTranslationUnit2(
				h_idx, "main.c"
				, 0, 0
				, files, 2
				, CXTranslationUnit_None, wr_tu) != CXError_Success
			|| !*wr_tu) {
		fputs("[forward] parse failed\n", stderr);
		return 1;
	}

	return 0;
}

static int s_dump(
		x_aclspv_ctx*		p_ctx,
		aclspv_wrd_t**		wr_spv,
		aclspv_wrdcount_t*	wr_count
		) {
	if(aclspv_dump_ctx_output(p_ctx, wr_count, wr_spv) || !*wr_spv) {
		fputs("[forward] dump failed\n", stderr);
		return 1;
	}

	return 0;
}

static void s_write(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	FILE* const fp = fopen(ACLTEST_OUT, "wb");

	if(!fp)
		return;

	fwrite(rd, sizeof(aclspv_wrd_t), (size_t)c, fp);
	fclose(fp);
}

/** forward reference: the compile phase resolves the whole chain itself. */
static int s_forward(void) {
	CXIndex			idx;
	CXTranslationUnit	tu = 0;
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	aclspv_wrd_t*		spv;
	aclspv_wrdcount_t	count;
	struct s_counts		c_compile;
	struct s_counts		c_build;
	int			ret = 1;

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[forward] index failed\n", stderr);
		return 1;
	}

	if(s_parse(s_fwd_lines,
				(unsigned)(sizeof(s_fwd_lines)
					/ sizeof(s_fwd_lines[0]))
				, idx, &tu))
		goto LBL_IDX;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[forward] init failed\n", stderr);
		goto LBL_TU;
	}

	/** phase 1: later, then caller (waiting on later), then the entry point
	 * (waiting on caller). all three land here. */
	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[forward] compile failed: %d\n", (int)ctx.m_err);
		goto LBL_CTX;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_CTX;

	if(s_counts(spv, count, &c_compile)) {
		free(spv);
		goto LBL_CTX;
	}

	printf("compile: fn=%u call=%u entp=%u\n"
			, c_compile.m_fn, c_compile.m_call, c_compile.m_entp);

	s_write(spv, count);
	free(spv);

	/** later, caller, and the entry point; caller->later + entry->caller. */
	if(c_compile.m_fn != 3
			|| c_compile.m_call != 2
			|| c_compile.m_entp != 1) {
		fputs("[forward] compile phase did not emit the whole chain\n", stderr);
		goto LBL_CTX;
	}

	/** phase 2: nothing was deferred, so the module must not change. */
	if(aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[forward] build failed: %d\n", (int)ctx.m_err);
		goto LBL_CTX;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_CTX;

	if(s_counts(spv, count, &c_build)) {
		free(spv);
		goto LBL_CTX;
	}

	printf("build: fn=%u call=%u entp=%u\n"
			, c_build.m_fn, c_build.m_call, c_build.m_entp);

	free(spv);

	if(c_build.m_fn != c_compile.m_fn
			|| c_build.m_call != c_compile.m_call
			|| c_build.m_entp != c_compile.m_entp) {
		fputs("[forward] build changed an already complete module\n", stderr);
		goto LBL_CTX;
	}

	ret = 0;

LBL_CTX:
	aclspv_fini_ctx(&ctx);

LBL_TU:
	clang_disposeTranslationUnit(tu);

LBL_IDX:
	clang_disposeIndex(idx);

	return ret;
}

/** recursion: the deferred set never empties, so the build phase reports it. */
static int s_recursive(void) {
	CXIndex			idx;
	CXTranslationUnit	tu = 0;
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	aclspv_compile_t	ret;
	int			rc = 1;

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[forward] index failed\n", stderr);
		return 1;
	}

	if(s_parse(s_rec_lines,
				(unsigned)(sizeof(s_rec_lines)
					/ sizeof(s_rec_lines[0]))
				, idx, &tu))
		goto LBL_IDX;

	memset(&cfg, 0, sizeof(cfg));

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[forward] init failed\n", stderr);
		goto LBL_TU;
	}

	/** a call cycle is rejected by the COMPILE phase: it is the phase that
	 * sees both bodies, and SPIR-V has no recursive OpFunctionCall for the
	 * build phase to complete. */
	ret = aclspv_ctx_compile1(&ctx, tu);

	printf("recursion: %d\n", (int)ret);

	if(ret == ACLSPV_COMPILE_RECURSIVE_CALL)
		rc = 0;
	else
		fprintf(stderr, "[forward] recursion compile: %d, want %d\n"
				, (int)ret, (int)ACLSPV_COMPILE_RECURSIVE_CALL);

	aclspv_fini_ctx(&ctx);

LBL_TU:
	clang_disposeTranslationUnit(tu);

LBL_IDX:
	clang_disposeIndex(idx);

	return rc;
}

int main(void) {
	acltestcfg();

	if(s_forward())
		return EXIT_FAILURE;

	if(s_recursive()) {
		fputs("[forward] recursion was not reported\n", stderr);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
