#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** `__builtin_shufflevector(v1, v2, i...)` -> OpVectorShuffle.
 *
 * the indices are literal components into v1 ++ v2, not value operands.
 * covers a same-width result and a narrower one (the result type follows
 * the mask length, not the operands), a nested shuffle as an operand, and a
 * shuffle feeding a vector arithmetic op.
 * */
static const char s_src_shufflevector[] =
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"typedef f32 __attribute__((ext_vector_type(2))) f32x2;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f32x4* a,\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f32x4* b,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32x4* out4,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32x2* out2\n"
") {\n"
"    *out4 = __builtin_shufflevector(*a, *b, 0, 5, 2, 7);\n"
"    *out4 = __builtin_shufflevector(__builtin_shufflevector(*a, *b, 0, 5, 2, 7), *a, 1, 6, 2, 0);\n"
"    *out4 = __builtin_shufflevector(*a, *b, 0, 5, 2, 7) + *a;\n"
"    *out2 = __builtin_shufflevector(*a, *b, 1, 6);\n"
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

	files[0].Contents	= s_src_shufflevector;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_shufflevector) - 1;

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
		fprintf(stderr, "[shufflevector] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[shufflevector] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[shufflevector] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u selmerge=%u brc=%u shuffle=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_selmerge
			, stats.m_num_brc
			, stats.m_num_shuffle
			, stats.m_targets_ok
			);

	if(stats.m_num_shuffle < 2 || !stats.m_targets_ok) {
		fputs("[shufflevector] OpVectorShuffle was not emitted\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[shufflevector] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
