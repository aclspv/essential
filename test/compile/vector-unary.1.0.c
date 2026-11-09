#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** `!`, `~` and `,` with vector operands.
 *
 *   ~v   -> OpNot, elementwise, result type = operand type
 *   !v   -> elementwise (v == 0) ? -1 : 0. for int4 the result is int4;
 *           for float4 it is int4 as well (Clang's ext_vector rule).
 *   (a,b)-> comma: vector-typed result is the right operand.
 * */
static const char s_src_vector_unary[] =
"typedef int i4 __attribute__((ext_vector_type(4)));\n"
"typedef float f4 __attribute__((ext_vector_type(4)));\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i4* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i4* b,\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f4* fin,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i4* out,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i4* fneg\n"
") {\n"
"    *out = ~*in;\n"
"    *out = !*in;\n"
"    *fneg = !*fin;\n"
"    *out = (*in, *b);\n"
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

	files[0].Contents	= s_src_vector_unary;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_vector_unary) - 1;

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
		fprintf(stderr, "[vector-unary] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[vector-unary] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[vector-unary] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u ieq=%u feq=%u bnot=%u select=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_ieq
			, stats.m_num_feq
			, stats.m_num_bnot
			, stats.m_num_select
			, stats.m_targets_ok
			);

	if(stats.m_num_ieq < 1
			|| stats.m_num_feq < 1
			|| stats.m_num_bnot < 1
			|| stats.m_num_select < 2
			|| !stats.m_targets_ok) {
		fputs("[vector-unary] vector !/~/, were not emitted correctly\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[vector-unary] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
