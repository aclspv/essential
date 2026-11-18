#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * exporting a function.
 *
 * `visibility("default")` exports; `visibility("hidden")` never does; a
 * plain declaration follows ACLSPV_CTX_FLAG0_EXPORT_BY_DEFAULT.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"__attribute__((visibility(\"default\")))\n",
	"i32 exported(i32 x) { return x + 1; }\n",
	"__attribute__((visibility(\"hidden\")))\n",
	"i32 hidden_fn(i32 x) { return x + 2; }\n",
	"i32 plain(i32 x) { return x + 3; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_export(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = exported(1) + hidden_fn(2) + plain(3);\n",
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

static int s_run(aclspv_wrd_t c_flag0, unsigned c_expected, aclspv_wrd_t** r_spv, aclspv_wrdcount_t* r_count) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	acltest_spirv_stats	stats;
	unsigned		c_link;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	files[0].Contents	= s_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(s_src);

	files[1].Contents	= s_oclbase;
	files[1].Filename	= "opencl-c-base.h";
	files[1].Length		= sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(
			files
			, 2
			, ARG
			, 1
			, r_count
			, r_spv
			, 0
			, c_flag0
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)*r_spv, *r_count);

	if(RET != ACLSPV_COMPILE_OK || !*r_spv) {
		fprintf(stderr, "[export] compile failed: %d\n", (int)RET);
		return 1;
	}

	if(acltest_spirv_scan(*r_spv, *r_count, &stats) != 0) {
		fputs("[export] malformed spir-v instruction stream\n", stderr);
		return 1;
	}

	c_link = s_count_linkage(*r_spv, *r_count);

	printf("linkage=%u expected=%u\n", c_link, c_expected);

	if(c_link != c_expected) {
		fprintf(stderr, "[export] linkage decorations: %u, expected %u\n"
				, c_link, c_expected);
		return 1;
	}

	return 0;
}

int main(void) {
	aclspv_wrd_t*		spv = 0;
	aclspv_wrdcount_t	count = 0;

	/** explicit export only: `exported`, not `hidden_fn` / `plain`. */
	if(s_run(0, 1, &spv, &count))
		return EXIT_FAILURE;

	free(spv);

	/** export-by-default: `exported` and `plain`, still not `hidden_fn`. */
	if(s_run(ACLSPV_CTX_FLAG0_EXPORT_BY_DEFAULT, 2, &spv, &count))
		return EXIT_FAILURE;

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[export] cannot write %s\n", ACLTEST_OUT);
			free(spv);
			return EXIT_FAILURE;
		}

		fwrite(spv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spv);
	return EXIT_SUCCESS;
}
