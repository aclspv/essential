#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** scalar f32 literal folding for the whole family.
 *
 * OpExtInst is not a legal OpSpecConstantOp, so a constant operand must fold
 * to a plain OpConstant. f16 is not folded yet; vectors have no fold.
 * */
static const char s_src_f32_math_literal[] =
"typedef float f32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32* out\n"
") {\n"
"    *out = __builtin_sinf(0.5f);\n"
"    *out = __builtin_cosf(0.5f);\n"
"    *out = __builtin_tanf(0.5f);\n"
"}\n";

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;

	acltestcfg();

	files[0].Contents	= s_src_f32_math_literal;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_f32_math_literal) - 1;

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
			, ae2f_NIL
			, ACLSPV_CTX_FLAG0_SYMBOL
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[f32-math-literal] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[f32-math-literal] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[f32-math-literal] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u extinst=%u sin=%u cos=%u tan=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_extinst
			, stats.m_num_sin
			, stats.m_num_cos
			, stats.m_num_tan
			, stats.m_targets_ok
			);

	if(stats.m_num_extinst
			|| stats.m_num_sin || stats.m_num_cos || stats.m_num_tan
			|| !stats.m_targets_ok) {
		fputs("[f32-math-literal] literal math was not folded\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[f32-math-literal] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
