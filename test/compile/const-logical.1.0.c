#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * a local constant chain, not direct literals:
 *   a = 1 && 0;  b = a && a;  c = b || a;
 * if the register value is tracked as literal/constant, the chain folds
 * without any runtime OpLogicalAnd / OpLogicalOr or branch.
 * */
static const char s_src_const_logical[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t_const(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    _Bool a = 1 && 0;\n"
"    _Bool b = a && a;\n"
"    _Bool c = b || a;\n"
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

	files[0].Contents	= s_src_const_logical;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_const_logical) - 1;

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
		fprintf(stderr, "[const-logical] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[const-logical] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[const-logical] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u selmerge=%u brc=%u land=%u lor=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_selmerge
			, stats.m_num_brc
			, stats.m_num_land
			, stats.m_num_lor
			, stats.m_targets_ok
			);

	if(stats.m_num_selmerge || stats.m_num_brc
			|| stats.m_num_land || stats.m_num_lor
			|| !stats.m_targets_ok) {
		fputs("[const-logical] constant chain did not fold\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[const-logical] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
