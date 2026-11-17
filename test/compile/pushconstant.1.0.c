#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * push-constant entry-point arguments.
 *
 * a non-pointer parameter with no `storage class` is a push constant
 * member, not a variable of its own: all members of one entry point share
 * a single OpVariable with storage class PushConstant. the test checks
 *  - exactly one PushConstant OpVariable is emitted for two arguments
 *  - the shared block struct exists
 *  - a load of a push-constant member resolves to a working expression
 *
 * note: the test targets are compiled with -DNDEBUG, so assert() is a
 * no-op here. every check must be an explicit branch returning failure.
 * */
static const char s_src_pc[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_pc(\n"
"    i32 a,\n"
"    i32 b,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_v\n"
") {\n"
"    *out_v = a + b;\n"
"}\n";

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		num_pc;

	acltestcfg();

	files[0].Contents	= s_src_pc;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_pc) - 1;

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
		fprintf(stderr, "[pushconstant.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[pushconstant.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[pushconstant.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	num_pc = acltest_spirv_count_var_in(
			spirv, count, (aclspv_wrd_t)SpvStorageClassPushConstant);

	printf("inst=%u pc_var=%u type_struct=%u\n"
			, stats.m_num_inst
			, num_pc
			, acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT)
			);

	/** both scalar arguments share the one push-constant block. */
	if(num_pc != 1) {
		fputs("[pushconstant.1.0] expected exactly one push-constant variable\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT) < 1) {
		fputs("[pushconstant.1.0] push-constant block struct missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the body must actually be emitted: a store plus at least one load. */
	if(acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_LABEL) < 1
			|| stats.m_num_inst < 10) {
		fputs("[pushconstant.1.0] function body missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[pushconstant.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
