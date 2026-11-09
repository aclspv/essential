#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** basic operators that used to be dropped silently:
 *
 *   !x        -> OpLogicalNot (over a bool-cast operand)
 *   ~x        -> OpNot
 *   (a, b)    -> evaluate a for its effects, yield b
 *
 * the comma case puts a side-effecting assignment on the left and a `!` on
 * the right: if the comma were dropped the right operand would never be
 * referenced, so the second OpLogicalNot disappears.
 * */
static const char s_src_unary_comma[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* y,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* x,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = !*in;\n"
"    *out = ~*in;\n"
"    *out = (*x = 5, !*y);\n"
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

	files[0].Contents	= s_src_unary_comma;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_unary_comma) - 1;

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
		fprintf(stderr, "[unary-comma] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[unary-comma] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[unary-comma] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u ieq=%u bnot=%u select=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_ieq
			, stats.m_num_bnot
			, stats.m_num_select
			, stats.m_targets_ok
			);

	if(stats.m_num_ieq < 2
			|| stats.m_num_bnot < 1
			|| !stats.m_targets_ok) {
		fputs("[unary-comma] !/~/, were not emitted correctly\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[unary-comma] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
