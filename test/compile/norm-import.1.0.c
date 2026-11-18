#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * an imported function: `ext_add` is declared and never defined here.
 *
 * the module must carry a declaration for it (OpFunction with no block),
 * decorate it with LinkageAttributes Import, request the Linkage capability,
 * and call it like any other function.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"i32 ext_add(i32 a, i32 b);\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_import(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = ext_add(2, 3);\n",
	"}\n",
};
/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	acltest_spirv_stats	stats;
	aclspv_wrdcount_t	count = 0;
	unsigned		c_call;
	unsigned		c_linkage;

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
			, &count
			, &spirv
			, 0
			, 0
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[import] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[import] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[import] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_call = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionCall);
	c_linkage = acltest_spirv_count_op(
			spirv, count, (unsigned)SpvOpDecorate);

	printf("call=%u decorate=%u fn=%u\n"
			, c_call, c_linkage, stats.m_num_inst);

	if(c_call != 1) {
		fprintf(stderr, "[import] calls: %u, expected 1\n", c_call);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the entry point's interface decorations plus the import's linkage. */
	if(c_linkage < 2) {
		fprintf(stderr, "[import] decorations: %u, expected at least 2\n"
				, c_linkage);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[import] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
