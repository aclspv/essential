#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * implicit bool -> int conversion on assignment:
 *   x = r;      (decl-ref assign lane)
 *   *out = r;   (deref-write lane)
 * both must materialise the conversion before aclcur_store_reg enforces
 * exact type equality.
 * */
static const char s_src_bool_assign[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t_bool_assign(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    _Bool r = *in != 0;\n"
"    i32 x = 0;\n"
"    x = r;\n"
"    *out = r;\n"
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

	files[0].Contents	= s_src_bool_assign;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_bool_assign) - 1;

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
		fprintf(stderr, "[bool-assign] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[bool-assign] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[bool-assign] malformed spir-v instruction stream\n", stderr);
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
		fputs("[bool-assign] unexpected control flow\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[bool-assign] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
