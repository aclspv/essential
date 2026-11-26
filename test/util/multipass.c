#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acltest/onefile.h>
#include <acltest/spirv.h>

/**
 * a context survives a translation unit.
 *
 * compile and build are not one-shot: a caller feeds one module from several
 * sources, each through its own translation unit. what a pass emitted keeps its
 * id, a body is never written twice, and a symbol that only an earlier pass
 * knew about does not leak into the next one.
 *
 * three things are pinned here:
 *  1. two compile+build passes over different sources produce one module with
 *     each function once, and the second pass can call into the first.
 *  2. a function a pass imported has no body to gain: a later pass that defines
 *     it reports ACLSPV_COMPILE_IMPORT_DEFINED instead of contradicting the
 *     `LinkageAttributes Import` the module already carries.
 *  3. a `static` function of an earlier source does not become a callee of a
 *     later one: the static cache is TU-local and is emptied between passes.
 * */

static const char* const s_first_lines[] = {
	"typedef int i32;\n",
	"i32 shared_add(i32 a, i32 b) { return a + b; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_first(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = shared_add(1, 2);\n",
	"}\n",
};

/** calls into the first source: `shared_add` has no body here. */
static const char* const s_second_lines[] = {
	"typedef int i32;\n",
	"i32 shared_add(i32 a, i32 b);\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_second(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = shared_add(3, 4);\n",
	"}\n",
};

/** the import the contradiction case declares. */
static const char* const s_importing_lines[] = {
	"typedef int i32;\n",
	"i32 ext_fn(i32 a);\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_importing(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = ext_fn(1);\n",
	"}\n",
};

static const char* const s_defining_ext_lines[] = {
	"typedef int i32;\n",
	"i32 ext_fn(i32 a) { return a; }\n",
};

/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[4][1024];

static void s_concat(char* wr, const char* const* rd_lines, unsigned c) {
	unsigned i;

	wr[0] = 0;

	for(i = 0; i < c; ++i)
		strcat(wr, rd_lines[i]);
}

/** count the instructions this test cares about, including how many distinct
 * OpFunction result ids the module uses. */
struct s_counts {
	unsigned	m_fn;
	unsigned	m_call;
	unsigned	m_entp;
	unsigned	m_dup_name;	/** OpEntryPoint entries naming one function */
};

static int s_count(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		struct s_counts*	wr
		) {
	aclspv_wrdcount_t	i = 5;
	unsigned		c_entp_seen = 0;
	aclspv_wrd_t		entp_ids[8];

	memset(wr, 0, sizeof(*wr));

	while(i < c) {
		const unsigned		wc = (unsigned)(rd[i] >> 16);
		const unsigned		op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 1;

		switch(op) {
			case (unsigned)SpvOpFunction:	++wr->m_fn;	break;

			case (unsigned)SpvOpFunctionCall:++wr->m_call;	break;

			case (unsigned)SpvOpEntryPoint:
				/** [op][execution model][id][name...]: the id is
				 * what must be unique. */
				if(c_entp_seen < sizeof(entp_ids) / sizeof(entp_ids[0])) {
					unsigned k;

					for(k = 0; k < c_entp_seen; ++k)
						if(entp_ids[k] == rd[i + 2])
							++wr->m_dup_name;

					entp_ids[c_entp_seen++] = rd[i + 2];
				}

				++wr->m_entp;
				break;

			default:
				break;
		}

		i += wc;
	}

	return 0;
}

static int s_parse(
		CXIndex			idx,
		const char*			c_name,
		CXTranslationUnit*		wr_tu
		) {
	struct CXUnsavedFile	files[1];

	files[0].Filename	= c_name;
	files[0].Contents	= s_src[0];
	files[0].Length		= (unsigned long)strlen(s_src[0]);

	if(clang_parseTranslationUnit2(
				idx, c_name
				, 0, 0
				, files, 1
				, CXTranslationUnit_None, wr_tu) != CXError_Success
			|| !*wr_tu) {
		fprintf(stderr, "[multipass] parse failed: %s\n", c_name);
		return 1;
	}

	return 0;
}

static int s_dump(
		x_aclspv_ctx*		ctx,
		aclspv_wrd_t**		wr_spv,
		aclspv_wrdcount_t*	wr_count,
		struct s_counts*	wr_c
		) {
	acltest_spirv_stats	stats;

	if(aclspv_dump_ctx_output(ctx, wr_count, wr_spv) || !*wr_spv) {
		fputs("[multipass] dump failed\n", stderr);
		return 1;
	}

	if(acltest_spirv_scan(*wr_spv, *wr_count, &stats) != 0) {
		fputs("[multipass] malformed spir-v instruction stream\n", stderr);
		free(*wr_spv);
		return 1;
	}

	if(s_count(*wr_spv, *wr_count, wr_c)) {
		fputs("[multipass] unreadable instruction stream\n", stderr);
		free(*wr_spv);
		return 1;
	}

	return 0;
}

/** two passes, one module: each function once, and the second source calls the
 * first one's body rather than importing it. */
static int s_case_twice(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	CXIndex		idx;
	CXTranslationUnit	tu;
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	count = 0;
	struct s_counts		C;
	int			rc = 1;

	s_concat(s_src[0], s_first_lines,
			(unsigned)(sizeof(s_first_lines) / sizeof(s_first_lines[0])));

	memset(&cfg, 0, sizeof(cfg));
	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[multipass] index failed\n", stderr);
		return 1;
	}

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[multipass] init failed\n", stderr);
		clang_disposeIndex(idx);
		return 1;
	}

	/** pass 1: shared_add and t_first. */
	if(s_parse(idx, "first.c", &tu))
		goto LBL_CTX;

	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[multipass] first pass failed: %d\n", (int)ctx.m_err);
		clang_disposeTranslationUnit(tu);
		goto LBL_CTX;
	}

	clang_disposeTranslationUnit(tu);

	s_concat(s_src[0], s_second_lines,
			(unsigned)(sizeof(s_second_lines) / sizeof(s_second_lines[0])));

	if(s_parse(idx, "second.c", &tu))
		goto LBL_CTX;

	/** pass 2 over another source, on the same context. */
	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[multipass] second pass failed: %d\n", (int)ctx.m_err);
		clang_disposeTranslationUnit(tu);
		goto LBL_CTX;
	}

	clang_disposeTranslationUnit(tu);

	if(s_dump(&ctx, &spv, &count, &C))
		goto LBL_CTX;

	printf("twice: fn=%u call=%u entp=%u dup=%u\n"
			, C.m_fn, C.m_call, C.m_entp, C.m_dup_name);

	/** shared_add + two entry points, three calls, no second body for
	 * shared_add and no second OpEntryPoint for either entry point. */
	if(C.m_fn != 3) {
		fprintf(stderr, "[multipass] functions: %u, expected 3\n", C.m_fn);
		goto LBL_FREE;
	}

	if(C.m_call != 2) {
		fprintf(stderr, "[multipass] calls: %u, expected 2\n", C.m_call);
		goto LBL_FREE;
	}

	if(C.m_entp != 2 || C.m_dup_name) {
		fprintf(stderr, "[multipass] entry points: %u, duplicated: %u\n"
				, C.m_entp, C.m_dup_name);
		goto LBL_FREE;
	}

	rc = 0;

LBL_FREE:
	free(spv);

LBL_CTX:
	aclspv_fini_ctx(&ctx);
	clang_disposeIndex(idx);

	return rc;
}

/** a definition of a function the module already imports is a contradiction. */
static int s_case_import_defined(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	CXIndex		idx;
	CXTranslationUnit	tu;
	aclspv_compile_t	ret;
	int			rc = 1;

	memset(&cfg, 0, sizeof(cfg));
	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[multipass] index failed\n", stderr);
		return 1;
	}

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[multipass] init failed\n", stderr);
		clang_disposeIndex(idx);
		return 1;
	}

	s_concat(s_src[0], s_importing_lines,
			(unsigned)(sizeof(s_importing_lines)
				/ sizeof(s_importing_lines[0])));

	if(s_parse(idx, "importing.c", &tu))
		goto LBL_CTX;

	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[multipass] import pass failed: %d\n", (int)ctx.m_err);
		clang_disposeTranslationUnit(tu);
		goto LBL_CTX;
	}

	clang_disposeTranslationUnit(tu);

	/** the same function, now with a body: the module already promised the
	 * linker that this definition comes from another module. */
	s_concat(s_src[0], s_defining_ext_lines,
			(unsigned)(sizeof(s_defining_ext_lines)
				/ sizeof(s_defining_ext_lines[0])));

	if(s_parse(idx, "defining.c", &tu))
		goto LBL_CTX;

	(void)aclspv_ctx_compile1(&ctx, tu);
	ret = aclspv_ctx_build1(&ctx, tu);

	clang_disposeTranslationUnit(tu);

	printf("import-defined: %d\n", (int)ret);

	if(ret == ACLSPV_COMPILE_IMPORT_DEFINED)
		rc = 0;
	else
		fputs("[multipass] a definition of an imported function was accepted\n"
				, stderr);

LBL_CTX:
	aclspv_fini_ctx(&ctx);
	clang_disposeIndex(idx);

	return rc;
}

/** a static of an earlier source is not a callee of a later one. */
static int s_case_static_gone(void) {
	static const char* const s_a[] = {
		"typedef int i32;\n",
		"static i32 hidden(i32 x) { return x; }\n",
		"i32 visible(i32 x) { return hidden(x); }\n",
	};
	static const char* const s_b[] = {
		"typedef int i32;\n",
		"i32 hidden(i32 x);\n",
		"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
		"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
		"void t_static(\n",
		"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
		") {\n",
		"    *out = hidden(1);\n",
		"}\n",
	};

	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	CXIndex		idx;
	CXTranslationUnit	tu;
	int			rc = 1;

	memset(&cfg, 0, sizeof(cfg));
	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[multipass] index failed\n", stderr);
		return 1;
	}

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[multipass] init failed\n", stderr);
		clang_disposeIndex(idx);
		return 1;
	}

	s_concat(s_src[0], s_a, (unsigned)(sizeof(s_a) / sizeof(s_a[0])));

	if(s_parse(idx, "a.c", &tu))
		goto LBL_CTX;

	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK
			|| aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[multipass] static pass failed: %d\n", (int)ctx.m_err);
		clang_disposeTranslationUnit(tu);
		goto LBL_CTX;
	}

	clang_disposeTranslationUnit(tu);

	/** the second source has its own, unrelated `hidden`. */
	s_concat(s_src[0], s_b, (unsigned)(sizeof(s_b) / sizeof(s_b[0])));

	if(s_parse(idx, "b.c", &tu))
		goto LBL_CTX;

	if(aclspv_ctx_compile1(&ctx, tu) == ACLSPV_COMPILE_OK
			&& aclspv_ctx_build1(&ctx, tu) == ACLSPV_COMPILE_OK)
		rc = 0;
	else
		fprintf(stderr, "[multipass] static pass 2 failed: %d\n"
				, (int)ctx.m_err);

	clang_disposeTranslationUnit(tu);

LBL_CTX:
	aclspv_fini_ctx(&ctx);
	clang_disposeIndex(idx);

	return rc;
}

int main(void) {
	acltestcfg();

	if(s_case_twice())		return EXIT_FAILURE;
	if(s_case_import_defined())	return EXIT_FAILURE;
	if(s_case_static_gone())	return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
