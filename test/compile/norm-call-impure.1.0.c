#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * an impure call invalidates the tracked register values.
 *
 * setv writes through its pointer parameter, so the module cannot own what
 * it touches: the read of *in after the call must be a fresh OpLoad, not the
 * value cached before it. a pure callee would leave the cache intact.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"void setv(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(12)\"))) i32* p,\n",
	"    i32 v\n",
	") { *p = v; }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_impure(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(12)\"))) i32* in,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    i32 before = *in;\n",
	"    setv(in, 7);\n",
	"    *out = before + *in;\n",
	"}\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_dead(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(12)\"))) i32* in,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    i32 unused = *in;\n",
	"    setv(in, 7);\n",
	"    *out = 1;\n",
	"}\n",
};
/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[4096];

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_call;
	unsigned		c_load;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	files[0].Contents	= s_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(s_src);

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
			, 0
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[norm-call-impure] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[norm-call-impure] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-call-impure] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_call = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionCall);
	c_load = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpLoad);

	printf("call=%u load=%u\n", c_call, c_load);

	if(c_call != 2) {
		fprintf(stderr, "[norm-call-impure] calls: %u, expected 2\n", c_call);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** t_impure: one load for the pre-call read, one for the post-call read
	 * (`in` is read again, so the clobbered reload is referred).
	 *
	 * t_dead: one load for the pre-call read only. the clobber materialises a
	 * reload for `in` there too, but nothing reads it afterwards, so aclcur
	 * drops it — a clobber load is not an instruction until it is referred.
	 */
	if(c_load != 3) {
		fprintf(stderr, "[norm-call-impure] loads: %u, expected 3\n"
				, c_load);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-call-impure] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
