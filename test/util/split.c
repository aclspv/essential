#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/spirv.h>

/**
 * the compile | build split, driven through a caller-owned translation unit.
 *
 * `wrapper` calls `imp`, which has no body: the compile phase must skip it
 * (and the entry point that calls it) and emit only `helper`. the build phase
 * declares the import and completes the two deferred bodies.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"i32 imp(i32 x);\n",
	"i32 wrapper(i32 x) { return imp(x); }\n",
	"i32 helper(i32 x) { return x + 1; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_split(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = helper(1) + wrapper(2);\n",
	"}\n",
};
/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

/** count LinkageAttributes decorations. */
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
		x_aclspv_ctx*		ctx,
		aclspv_wrd_t**		wr_spv,
		aclspv_wrdcount_t*	wr_count
		) {
	if(aclspv_dump_ctx_output(ctx, wr_count, wr_spv) || !*wr_spv) {
		fputs("[split] dump failed\n", stderr);
		return 1;
	}

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
	unsigned		c_fn;
	unsigned		c_call;
	unsigned		c_link;
	unsigned		c_entp;
	int			c_linkcap;
	acltest_spirv_stats	stats;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	memset(&cfg, 0, sizeof(cfg));
	files[0].Filename	= "main.c";
	files[0].Contents	= s_src;
	files[0].Length		= (unsigned long)strlen(s_src);

	idx = clang_createIndex(0, 0);

	if(!idx) {
		fputs("[split] index failed\n", stderr);
		return EXIT_FAILURE;
	}

	if(clang_parseTranslationUnit2(
				idx, "main.c"
				, 0, 0
				, files, 1
				, CXTranslationUnit_None, &tu) != CXError_Success
			|| !tu) {
		fputs("[split] parse failed\n", stderr);
		clang_disposeIndex(idx);
		return EXIT_FAILURE;
	}

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[split] init failed\n", stderr);
		clang_disposeTranslationUnit(tu);
		clang_disposeIndex(idx);
		return EXIT_FAILURE;
	}

	/** phase 1: only `helper` is import-free. */
	if(aclspv_ctx_compile1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[split] compile failed: %d\n", (int)ctx.m_err);
		goto LBL_FAIL;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_FAIL;

	if(acltest_spirv_scan(spv, count, &stats) != 0) {
		fputs("[split] malformed spir-v instruction stream\n", stderr);
		free(spv);
		goto LBL_FAIL;
	}

	c_fn	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunctionCall);
	c_link	= s_count_linkage(spv, count);
	c_entp	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpEntryPoint);
	c_linkcap = s_has_capability(
			spv, count, (aclspv_wrd_t)SpvCapabilityLinkage);

	printf("compile: fn=%u call=%u linkage=%u entp=%u linkcap=%d\n"
			, c_fn, c_call, c_link, c_entp, c_linkcap);

	free(spv);

	/** no entry point yet, so the module is only legal under Linkage. */
	if(c_fn != 1 || c_call != 0 || c_link || c_entp || !c_linkcap) {
		fputs("[split] compile phase emitted a deferred body\n", stderr);
		goto LBL_FAIL;
	}

	/** phase 2: the import declaration and the deferred bodies. the TU is
	 * still alive — that is the contract compile1 documents. */
	if(aclspv_ctx_build1(&ctx, tu) != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[split] build failed: %d\n", (int)ctx.m_err);
		goto LBL_FAIL;
	}

	if(s_dump(&ctx, &spv, &count))
		goto LBL_FAIL;

	if(acltest_spirv_scan(spv, count, &stats) != 0) {
		fputs("[split] malformed spir-v instruction stream\n", stderr);
		free(spv);
		goto LBL_FAIL;
	}

	c_fn	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spv, count, (unsigned)SpvOpFunctionCall);

	printf("build: fn=%u call=%u linkage=%u\n"
			, c_fn, c_call, s_count_linkage(spv, count));

	/** the import declaration, helper, wrapper, and the entry point.
	 * calls: wrapper->imp, entry->helper, entry->wrapper. */
	if(c_fn != 4 || c_call != 3 || s_count_linkage(spv, count) != 1) {
		fputs("[split] build did not complete the module\n", stderr);
		free(spv);
		goto LBL_FAIL;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[split] cannot write %s\n", ACLTEST_OUT);
			free(spv);
			goto LBL_FAIL;
		}

		fwrite(spv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spv);
	aclspv_fini_ctx(&ctx);
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(idx);
	return EXIT_SUCCESS;

LBL_FAIL:
	aclspv_fini_ctx(&ctx);
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(idx);
	return EXIT_FAILURE;
}
