#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** `__builtin_elementwise_cos` / `__builtin_elementwise_tan` on f32x4.
 *
 * the elementwise spellings are generic: their type comes from the operand,
 * so they share the same carve lane as the fixed f32/f16 spellings. */
static const char s_src_elementwise_cos_tan[] =
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f32x4* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32x4* out\n"
") {\n"
"    *out = __builtin_elementwise_cos(*in);\n"
"    *out = __builtin_elementwise_tan(*in);\n"
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

	files[0].Contents	= s_src_elementwise_cos_tan;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_elementwise_cos_tan) - 1;

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
		fprintf(stderr, "[elementwise-cos-tan] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[elementwise-cos-tan] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[elementwise-cos-tan] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u extinst=%u cos=%u tan=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_extinst
			, stats.m_num_cos
			, stats.m_num_tan
			, stats.m_targets_ok
			);

	if(stats.m_num_cos < 1 || stats.m_num_tan < 1 || !stats.m_targets_ok) {
		fputs("[elementwise-cos-tan] cos/tan were not emitted\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[elementwise-cos-tan] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
