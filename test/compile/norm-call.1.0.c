#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * calls between normal functions and from an entry point.
 *
 * add is called by twice, twice by the entry point: three OpFunction and two
 * OpFunctionCall. the module must validate, so the opcodes and the parameter
 * types have to line up exactly.
 * */
static const char s_src_norm_call[] =
"typedef int i32;\n"
"i32 add(i32 a, i32 b) { return a + b; }\n"
"i32 twice(i32 x) { return add(x, x); }\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_call(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = twice(3);\n"
"}\n";

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_fn;
	unsigned		c_call;

	acltestcfg();

	files[0].Contents	= s_src_norm_call;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_norm_call) - 1;

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
		fprintf(stderr, "[norm-call] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[norm-call] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-call] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_fn = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);
	c_call = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionCall);

	printf("fn=%u call=%u\n", c_fn, c_call);

	if(c_fn != 3) {
		fprintf(stderr, "[norm-call] functions: %u, expected 3\n", c_fn);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(c_call != 2) {
		fprintf(stderr, "[norm-call] calls: %u, expected 2\n", c_call);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-call] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
