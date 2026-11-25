#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * workgroup storage class on entry-point arguments.
 *
 * the point of this test is the branch, not the numeric result:
 *  - every `workgroup` argument is an OpVariable with storage class
 *    Workgroup (not Input/Output/Private)
 *  - the vulkan block idiom wraps the pointee, so the wrapper structs
 *    exist in the type section
 *
 * note: the test targets are compiled with -DNDEBUG, so assert() is a
 * no-op here. every check must be an explicit branch returning failure.
 * */
static const char* const s_src_wg_lines[] = {
	"typedef int i32;\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_wg(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(4)\"))) i32* w_out,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(4)\"))) i32* w_scratch,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in_n,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_v\n",
	") {\n",
	"    i32 t = *in_n;\n",
	"    w_scratch[0] = t + 1;\n",
	"    w_out[0] = w_scratch[0] * 2;\n",
	"    *out_v = w_out[0];\n",
	"}\n",
};
/** C90 caps a concatenated string literal at 509 characters: the source is
 * assembled at runtime instead. */
static char s_src_wg[4096];

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		num_wg;

	acltestcfg();
	acltest_concat(s_src_wg, s_src_wg_lines,
			(unsigned)(sizeof(s_src_wg_lines) / sizeof(s_src_wg_lines[0])));

	files[0].Contents	= s_src_wg;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(s_src_wg);

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
		fprintf(stderr, "[workgroup.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[workgroup.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[workgroup.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	num_wg = acltest_spirv_count_var_in(
			spirv, count, (aclspv_wrd_t)SpvStorageClassWorkgroup);

	printf("inst=%u wg_var=%u type_struct=%u\n"
			, stats.m_num_inst
			, num_wg
			, acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT)
			);

	/** two workgroup arguments, both emitted as Workgroup variables. */
	if(num_wg != 2) {
		fputs("[workgroup.1.0] workgroup arguments did not become workgroup variables\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the block wrapper of a pointer argument. both workgroup arguments
	 * point at the same builtin type, so the wrapper struct is deduped to
	 * a single type node. */
	if(acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT) < 1) {
		fputs("[workgroup.1.0] block wrapper structs missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** three statements write memory — `w_scratch[0]`, `w_out[0]` and
	 * `*out_v` — so three OpStores. this mattered: a workgroup pointer
	 * argument is one `struct { T }` element, and a `w[i]` whose registered
	 * member 0 is a BUILTIN type (no aclty node) used to be rejected by the
	 * subscript carve and then dropped in silence, leaving the module with
	 * the reads only. */
	if(acltest_spirv_count_op(spirv, count, (unsigned)SpvOpStore) != 3) {
		fprintf(stderr
				, "[workgroup.1.0] stores: %u, expected 3 "
				  "(w_scratch[0], w_out[0], *out_v)\n"
				, acltest_spirv_count_op(spirv, count, (unsigned)SpvOpStore));
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[workgroup.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
