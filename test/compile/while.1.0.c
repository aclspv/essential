#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * a loop with a value-phi carry and both loop-control statements.
 * the point of this test is structure, not the numerical result:
 *  - OpLoopMerge must exist (while was carved as a loop)
 *  - OpBranchConditional must exist (condition + ifs)
 *  - every branch / merge target must name a defined OpLabel
 *
 * note: the test targets are compiled with -DNDEBUG, so assert() is a
 * no-op here. every check must be an explicit branch returning failure.
 * */
static const char s_src_while[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_while(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in_n,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_sum\n"
") {\n"
"    i32 i = 0;\n"
"    i32 sum = 0;\n"
"    while (i < *in_n) {\n"
"        i = i + 1;\n"
"        if (i == 3) { continue; }\n"
"        if (i == 7) { break; }\n"
"        sum = sum + i;\n"
"    }\n"
"    *out_sum = sum;\n"
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

	files[0].Contents	= s_src_while;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_while) - 1;

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
		fprintf(stderr, "[while.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[while.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[while.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u lbl=%u br=%u brc=%u selmerge=%u loopmerge=%u phi=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_lbl
			, stats.m_num_br
			, stats.m_num_brc
			, stats.m_num_selmerge
			, stats.m_num_loopmerge
			, stats.m_num_phi
			, stats.m_targets_ok
			);

	if(!stats.m_num_lbl
			|| stats.m_num_loopmerge < 1
			|| stats.m_num_brc < 1
			|| stats.m_num_phi < 1
			|| !stats.m_targets_ok) {
		fputs("[while.1.0] loop control flow missing or targets invalid\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[while.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
