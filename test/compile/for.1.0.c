#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * `for` statements in every header shape:
 *
 *  - init + cond + inc, with both loop-control statements in the body
 *  - init only / cond only / inc only / no part at all
 *  - nested loops (the inner header's entry resolution used to recurse
 *    forever between the two headers)
 *  - a body that can only leave through break: its increment block has no
 *    live predecessor and must not emit reads from a register the loop
 *    carries (there is no memory to load it from)
 *
 * the point of this test is structure, not the numerical result:
 *  - one OpLoopMerge per loop (8 here)
 *  - OpBranchConditional for every condition
 *  - every branch / merge target must name a defined OpLabel
 *
 * note: the test targets are compiled with -DNDEBUG, so assert() is a
 * no-op here. every check must be an explicit branch returning failure.
 * */
static const char* const s_src_for_lines[] = {
"typedef int i32;\n",
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
"void t_for(\n",
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in_n,\n",
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_sum\n",
") {\n",
"    i32 sum = 0;\n",
"    i32 k = 0;\n",
"\n",
"    for (i32 i = 0; i < *in_n; i = i + 1) {\n",
"        if (i == 3) { continue; }\n",
"        if (i == 7) { break; }\n",
"        sum = sum + i;\n",
"    }\n",
"\n",
"    for (k = 0; ; ) {\n",
"        if (k > 0) { break; }\n",
"        k = k + 1;\n",
"    }\n",
"\n",
"    for (; k < 2; ) { k = k + 1; }\n",
"\n",
"    for (; ; k = k + 1) {\n",
"        if (k > 3) { break; }\n",
"    }\n",
"\n",
"    for (;;) {\n",
"        if (k == 0) { break; }\n",
"        k = k - 1;\n",
"    }\n",
"\n",
"    for (i32 a = 0; a < 2; a = a + 1) {\n",
"        for (i32 b = 0; b < 2; b = b + 1) {\n",
"            sum = sum + a + b;\n",
"        }\n",
"    }\n",
"\n",
"    for (i32 d = 0; d < 2; d = d + 1) { break; }\n",
"\n",
"    *out_sum = sum + k;\n",
"}\n"
};

static char s_src_for[2048];

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;

	acltestcfg();

	acltest_concat(s_src_for, s_src_for_lines,
			(unsigned)(sizeof(s_src_for_lines) / sizeof(s_src_for_lines[0])));

	files[0].Contents	= s_src_for;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(s_src_for);

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
			, ACLSPV_CTX_FLAG0_SYMBOL
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[for.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[for.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[for.1.0] malformed spir-v instruction stream\n", stderr);
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
			|| stats.m_num_loopmerge < 8
			|| stats.m_num_brc < 8
			|| stats.m_num_phi < 1
			|| !stats.m_targets_ok) {
		fputs("[for.1.0] loop control flow missing or targets invalid\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[for.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
