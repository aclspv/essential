#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * uniform (uniform buffer) entry-point argument with an explicit
 * descriptor set and binding.
 *
 * this covers the storage class the existing tests never touch, plus the
 * attribute paths that are unique to bindables:
 *  - the argument becomes one OpVariable with storage class Uniform
 *  - its type is a Block wrapper struct (std140 keeps the two i32 members
 *    at offsets 0 and 4)
 *  - `aclspv_set` / `aclspv_bind_idx` are parsed and emitted as
 *    DescriptorSet / Binding decorations
 *
 * note: the test targets are compiled with -DNDEBUG, so assert() is a
 * no-op here. every check must be an explicit branch returning failure.
 * */
static const char s_src_ubo[] =
"typedef int i32;\n"
"typedef struct { i32 m_a; i32 m_b; } pair_t;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_ubo(\n"
"    __attribute__((annotate(\"aclspv_set(2)\")))\n"
"    __attribute__((annotate(\"aclspv_bind_idx(5)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(2)\")))\n"
"    pair_t* u,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_v\n"
") {\n"
"    *out_v = u->m_a + u->m_b;\n"
"}\n";

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		num_ubo;
	unsigned		num_decorate;

	acltestcfg();

	files[0].Contents	= s_src_ubo;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_ubo) - 1;

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
		fprintf(stderr, "[uniform.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[uniform.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[uniform.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	num_ubo		= acltest_spirv_count_var_in(
				spirv, count, (aclspv_wrd_t)SpvStorageClassUniform);
	num_decorate	= acltest_spirv_count_op(
				spirv, count, ACLTEST_SPV_OP_DECORATE);

	printf("inst=%u ubo_var=%u decorate=%u\n"
			, stats.m_num_inst
			, num_ubo
			, num_decorate
			);

	if(num_ubo != 1) {
		fputs("[uniform.1.0] uniform argument did not become a uniform variable\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT) < 1) {
		fputs("[uniform.1.0] uniform block struct missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** DescriptorSet + Binding, plus whatever the type needs. */
	if(num_decorate < 2) {
		fputs("[uniform.1.0] descriptor set / binding decorations missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[uniform.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
