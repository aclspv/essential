#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** `c ? t : f` with at least one impure arm -> branch + phi, never OpSelect.
 *
 * when either arm has a side effect, evaluating both is wrong, so the
 * expression lowers to a conditional branch whose blocks each evaluate one
 * arm; a phi at the join selects the value. covers parenthesised arms (a
 * wrapper cursor re-enters at the same operand index), a pure literal arm
 * against an impure one, and a nested ternary in the true arm.
 * */
static const char s_src_ternary_impure[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* x,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* y,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* z,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = *in ? ((*x = 1)) : ((*y = 2));\n"
"    *out = *in ? 3 : (*y = 2);\n"
"    *out = *in ? (*in ? (*x = 1) : (*y = 2)) : (*z = 3);\n"
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

	files[0].Contents	= s_src_ternary_impure;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_ternary_impure) - 1;

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
		fprintf(stderr, "[ternary-impure] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[ternary-impure] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[ternary-impure] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u selmerge=%u brc=%u phi=%u select=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_selmerge
			, stats.m_num_brc
			, stats.m_num_phi
			, stats.m_num_select
			, stats.m_targets_ok
			);

	if(stats.m_num_select
			|| stats.m_num_selmerge < 1
			|| stats.m_num_brc < 1
			|| stats.m_num_phi < 1
			|| !stats.m_targets_ok) {
		fputs("[ternary-impure] expected a branch/phi lane\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[ternary-impure] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
