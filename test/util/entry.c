#include <aclspv.h>

#include <acltest/onefile.h>
#include <acltest/spirv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * entry points and normal functions across the compile | build split.
 *
 * `helper` is import-free, `wrapper` calls an import (`imp`), and the two
 * kernels are placed on both sides of the split:
 *   t_pure  calls helper            -> emit in the compile phase
 *   t_imp   calls wrapper and imp   -> emit in the build phase
 * so the compile module has one entry point, one normal body and no import,
 * and the build phase must add the import declaration, `wrapper`, and the
 * second entry point with all its decorations.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"i32 imp(i32 x);\n",
	"i32 helper(i32 x) { return x + 1; }\n",
	"i32 wrapper(i32 x) { return imp(x) * 2; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(4, 2, 1)\")))\n",
	"void t_pure(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = helper(1);\n",
	"}\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(8, 1, 1)\")))\n",
	"void t_imp(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = wrapper(2) + imp(3);\n",
	"}\n",
};

/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[2048];

static unsigned s_count_linkage(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c
		) {
	aclspv_wrdcount_t	i = 5;
	unsigned		n = 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			break;

		if((unsigned)(rd[i] & 0xFFFFu) == (unsigned)SpvOpDecorate
				&& wc >= 3
				&& (unsigned)(rd[i + 2] & 0xFFFFu)
					== (unsigned)SpvDecorationLinkageAttributes)
			++n;

		i += wc;
	}

	return n;
}

struct s_counts {
	unsigned	m_fn;
	unsigned	m_call;
	unsigned	m_entp;
	unsigned	m_mode;
	unsigned	m_link;
	int		m_scan;
};

static int s_count(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		struct s_counts*	wr
		) {
	acltest_spirv_stats stats;

	wr->m_scan = acltest_spirv_scan(rd, c, &stats);

	if(wr->m_scan)
		return 1;

	wr->m_fn	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpFunction);
	wr->m_call	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpFunctionCall);
	wr->m_entp	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpEntryPoint);
	wr->m_mode	= acltest_spirv_count_op(
			rd, c, (unsigned)SpvOpExecutionMode);
	wr->m_link	= s_count_linkage(rd, c);

	return 0;
}

static void s_print(const char* c_tag, const struct s_counts* c) {
	printf("%s: fn=%u call=%u entp=%u mode=%u linkage=%u\n"
			, c_tag, c->m_fn, c->m_call, c->m_entp, c->m_mode, c->m_link);
}

static int s_write(
		const char*		c_path,
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c
		) {
	FILE* const fp = fopen(c_path, "wb");

	if(!fp)
		return 1;

	fwrite(rd, sizeof(aclspv_wrd_t), (size_t)c, fp);
	fclose(fp);

	return 0;
}

int main(void) {
	CXIndex			idx;
	CXTranslationUnit	tu = 0;
	struct CXUnsavedFile	files[1];
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	aclspv_wrd_t*		spv;
	aclspv_wrdcount_t	count;
	struct s_counts		c_compile;
	struct s_counts		c_build;
	int			rc = EXIT_FAILURE;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));
	memset(&cfg, 0, sizeof(cfg));

	files[0].Filename	= "main.c";
	files[0].Contents	= s_src;
	files[0].Length		= (unsigned long)strlen(s_src);

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[entry] index failed\n", stderr);
		return EXIT_FAILURE;
	}

	if(clang_parseTranslationUnit2(
				idx, "main.c"
				, 0, 0
				, files, 1
				, CXTranslationUnit_None, &tu) != CXError_Success
			|| !tu) {
		fputs("[entry] parse failed\n", stderr);
		clang_disposeIndex(idx);
		return EXIT_FAILURE;
	}

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[entry] init failed\n", stderr);
		clang_disposeTranslationUnit(tu);
		clang_disposeIndex(idx);
		return EXIT_FAILURE;
	}

	/** compile: helper and t_pure go out; wrapper and t_imp wait. */
	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[entry] compile failed: %d\n", (int)ctx.m_err);
		goto LBL_FAIL;
	}

	if(aclspv_dump_ctx_output(&ctx, &count, &spv) || !spv) {
		fputs("[entry] compile dump failed\n", stderr);
		goto LBL_FAIL;
	}

	if(s_count(spv, count, &c_compile)) {
		fputs("[entry] compile module malformed\n", stderr);
		free(spv);
		goto LBL_FAIL;
	}

	s_print("compile", &c_compile);
	free(spv);

	if(c_compile.m_fn != 2 || c_compile.m_call != 1
			|| c_compile.m_entp != 1 || c_compile.m_mode != 1
			|| c_compile.m_link != 0) {
		fputs("[entry] compile phase emitted the wrong set\n", stderr);
		goto LBL_FAIL;
	}

	/** build: the import, wrapper, and the second entry point. */
	if(aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[entry] build failed: %d\n", (int)ctx.m_err);
		goto LBL_FAIL;
	}

	if(aclspv_dump_ctx_output(&ctx, &count, &spv) || !spv) {
		fputs("[entry] build dump failed\n", stderr);
		goto LBL_FAIL;
	}

	if(s_count(spv, count, &c_build)) {
		fputs("[entry] build module malformed\n", stderr);
		free(spv);
		goto LBL_FAIL;
	}

	s_print("build", &c_build);
	s_write(ACLTEST_OUT, spv, count);
	free(spv);

	/** import declaration, helper, wrapper, t_pure, t_imp; calls:
	 * t_pure->helper, wrapper->imp, t_imp->wrapper + t_imp->imp. */
	if(c_build.m_fn != 5 || c_build.m_call != 4
			|| c_build.m_entp != 2 || c_build.m_mode != 2
			|| c_build.m_link != 1) {
		fputs("[entry] build did not complete the module\n", stderr);
		goto LBL_FAIL;
	}

	rc = EXIT_SUCCESS;

LBL_FAIL:
	aclspv_fini_ctx(&ctx);
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(idx);

	return rc;
}
