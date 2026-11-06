#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

#if 1
int main(void);
int main(void) {
	struct CXUnsavedFile files[2];
	const char *ARG[] = {"main.c"};
	aclspv_compile_t RET;

	aclspv_wrd_t *spirv0;
	aclspv_wrdcount_t spirv_count;
	acltest_spirv_stats stats;

	acltestcfg();

	files[0].Contents = s_src0;
	files[0].Filename = "main.c";
	files[0].Length = sizeof(s_src0) - 1;

	puts("main.cl::::::::::::::::::");
	puts(files[0].Contents);
	puts("main.cl::::::::::::::::::");

	files[1].Contents = s_oclbase;
	files[1].Filename = "opencl-c-base.h";
	files[1].Length = sizeof(s_oclbase) - 1;

	puts("opencl-c-base.h::::::::::::::::::");
	puts(files[1].Contents);
	puts("opencl-c-base.h::::::::::::::::::");

	RET = dbg_aclspv_compile(
			files
			, 2
			, ARG
			, 1
			, &spirv_count
			, &spirv0
			, ae2f_NIL
			, ACLSPV_CTX_FLAG0_SYMBOL
			);

	printf("aclspv_compile: %d\n", (int) RET);
	printf("output spirv: %p, %u\n", (void *)spirv0, spirv_count);

	/** the test targets are compiled with -DNDEBUG: make every check a
	 * real failure, not an assert that the preprocessor erases. */
	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[gl-simple.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv0) {
		fputs("[gl-simple.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	/** the shared source has if/else entry points; a successful compile is
	 * not enough, the emitted CFG must carry selections and every merge /
	 * branch target must name a defined label. */
	if(acltest_spirv_scan(spirv0, spirv_count, &stats) != 0) {
		fputs("[gl-simple.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv0);
		return EXIT_FAILURE;
	}

	printf("inst=%u lbl=%u br=%u brc=%u selmerge=%u loopmerge=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_lbl
			, stats.m_num_br
			, stats.m_num_brc
			, stats.m_num_selmerge
			, stats.m_num_loopmerge
			, stats.m_targets_ok
			);

	if(stats.m_num_selmerge < 1 || !stats.m_targets_ok) {
		fputs("[gl-simple.1.0] selection control flow missing or targets invalid\n", stderr);
		free(spirv0);
		return EXIT_FAILURE;
	}

	if (spirv0) {
		FILE *const fp = fopen("result-c-gl-simple-1.0.spv", "wb");
		unless(fp) goto FPOPEN_FAILED;

		fwrite(spirv0, sizeof(aclspv_wrd_t), (size_t)spirv_count, fp);

	FPOPEN_FAILED:
		if(fp) fclose(fp);
	}

	puts("freeing spirv");
	free(spirv0);

	return EXIT_SUCCESS;
}
#else

#endif
