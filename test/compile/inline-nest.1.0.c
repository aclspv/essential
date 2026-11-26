#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>
#include <ae2f/Keys.h>

/**
 * an inlined body carved INTO an inlined body: the nested single-iteration
 * loop scaffolds of the inliner.
 *
 * every call here is `inline` with ACLSPV_CTX_FLAG0_INLINE_BY_DEFAULT on, so
 * `clamp_both` is carved at both of its call sites and, in turn, carves
 * `clamp_lo` twice inside each of them. three scaffolds deep, the registers
 * of one scaffold must never be served as the value of another's block: the
 * module used to come out with an OpPhi naming a value defined in a loop that
 * does not dominate the phi's block, which spirv-val rejects.
 *
 * the shape is small on purpose: only the nesting matters.
 * */
static const char* const s_src_lines[] = {
"typedef float f32;\n",
"inline f32 clamp_lo(f32 x) {\n",
"    if (x < 0.0f) return 0.0f;\n",
"    return x;\n",
"}\n",
"inline f32 clamp_both(f32 x) {\n",
"    f32 c = clamp_lo(x);\n",
"    return clamp_lo(c);\n",
"}\n",
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n",
"void entry(__attribute__((annotate(\"aclspv_storage_class(3)\"))) f32* out) {\n",
"    f32 acc = 0.25f;\n",
"    f32 once  = clamp_both(acc);\n",
"    f32 twice = clamp_both(once);\n",
"    *out = acc + twice;\n",
"}\n"
};

static char s_src[1024];

int main(void) {
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.c"};
	aclspv_compile_t RET;
	aclspv_wrd_t* spirv = 0;
	aclspv_wrdcount_t count = 0;
	unsigned calls;
	acltest_spirv_stats stats;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	files[0].Contents = s_src;
	files[0].Filename = "main.c";
	files[0].Length = (unsigned)strlen(s_src);

	files[1].Contents = s_oclbase;
	files[1].Filename = "opencl-c-base.h";
	files[1].Length = sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(files, 2, ARG, 1, &count, &spirv, 0,
			ACLSPV_CTX_FLAG0_INLINE_BY_DEFAULT);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fprintf(stderr, "[inline-nest] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats)) {
		fputs("[inline-nest] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	calls = acltest_spirv_count_op(spirv, count,
			(unsigned)ACLTEST_SPV_OP_FUNCTION_CALL);

	/** every callee here is `inline` and the flag carves them: nothing is
	 * left as an OpFunctionCall. */
	if(calls != 0) {
		fprintf(stderr, "[inline-nest] unexpected OpFunctionCall count: %u\n"
				, calls);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[inline-nest] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
