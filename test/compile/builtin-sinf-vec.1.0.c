#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** vector `__builtin_elementwise_sin` -> GLSL.std.450 Sin on f32x4. */
static const char s_src_builtin_sinf_vec[] =
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f32x4* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32x4* out\n"
") {\n"
"    *out = __builtin_elementwise_sin(*in);\n"
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

	files[0].Contents	= s_src_builtin_sinf_vec;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_builtin_sinf_vec) - 1;

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
		fprintf(stderr, "[builtin-sinf-vec] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[builtin-sinf-vec] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[builtin-sinf-vec] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u selmerge=%u brc=%u extinst=%u sin=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_selmerge
			, stats.m_num_brc
			, stats.m_num_extinst
			, stats.m_num_sin
			, stats.m_targets_ok
			);

	if(stats.m_num_selmerge || stats.m_num_brc
			|| stats.m_num_sin < 1
			|| !stats.m_targets_ok) {
		fputs("[builtin-sinf-vec] Sin was not emitted\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[builtin-sinf-vec] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
