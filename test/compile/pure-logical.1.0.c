#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * both sides of `&&` / `||` are pure (loads), so short-circuiting is not
 * observable. the before-visitor must flatten them to a single
 * OpLogicalAnd / OpLogicalOr: no selection merge, no conditional branch.
 * */
static const char s_src_pure_logical[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_pure_logical(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* a,\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* b,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    _Bool ba = *a;\n"
"    _Bool bb = *b;\n"
"    i32 x = ba && bb;\n"
"    i32 y = ba || bb;\n"
"    i32 z = 1 && 2;\n"
"    i32 w = 1 || 0;\n"
"    *out = x + y + z + w;\n"
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

	files[0].Contents	= s_src_pure_logical;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_pure_logical) - 1;

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
		fprintf(stderr, "[pure-logical] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[pure-logical] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[pure-logical] malformed spir-v instruction stream\n", stderr);
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
			|| stats.m_num_land < 1 || stats.m_num_lor < 1
			|| !stats.m_targets_ok) {
		fputs("[pure-logical] pure logical expression was not flattened\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[pure-logical] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
