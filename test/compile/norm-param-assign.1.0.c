#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * assigning to a parameter.
 *
 * a parameter is an immutable SSA value, so `x = ...` needs a local variable
 * initialised from the parameter at the entry. covers a straight-line
 * assignment and a branch-local one (the join must resolve the unassigned
 * path to the initial value).
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"i32 bump(i32 x) { x = x + 1; return x; }\n",
	"i32 cond(i32 x) {\n",
	"    if (x) { x = 7; }\n",
	"    return x;\n",
	"}\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_param(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = bump(4) + cond(1);\n",
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
	unsigned		c_fn;
	unsigned		c_call;

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
		fprintf(stderr, "[param] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[param] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[param] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_fn = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);
	c_call = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionCall);

	printf("fn=%u call=%u phi=%u\n", c_fn, c_call, stats.m_num_phi);

	/** bump, cond, and the entry point. */
	if(c_fn != 3) {
		fprintf(stderr, "[param] functions: %u, expected 3\n", c_fn);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(c_call != 2) {
		fprintf(stderr, "[param] calls: %u, expected 2\n", c_call);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[param] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
