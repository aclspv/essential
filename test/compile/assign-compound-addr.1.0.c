#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * compound assignment on a variable whose address has been taken.
 *
 *   *v += 2    -> view-rooted l-value store-back
 *   aa += 34   -> plain-variable store-back
 *
 * the l-value operand's read is what names the store target; both shapes used
 * to fail when the result extra (which never saw the l-value) was handed to
 * the store-back, and when the pre-allocated result id of a nil-operand
 * arithmetic was treated as a value.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"i32 a(i32*);\n",
	"i32 b(i32* v) { *v += 2; return a(v); }\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"void main(void) {\n",
	"    i32 aa = 0;\n",
	"    i32 pa = b(&aa);\n",
	"    aa += 34;\n",
	"}\n",
	"i32 a(i32* p) { return *p + '0'; }\n",
};

/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_fn;
	unsigned		c_call;
	unsigned		c_store;

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
			, ACLSPV_CTX_FLAG0_SYMBOL
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fprintf(stderr, "[assign-compound-addr] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[assign-compound-addr] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_fn	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);
	c_call	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionCall);
	c_store	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpStore);

	printf("fn=%u call=%u store=%u\n", c_fn, c_call, c_store);

	/** a, b, main; b->a and main->b. */
	if(c_fn != 3 || c_call != 2) {
		fprintf(stderr, "[assign-compound-addr] fn=%u call=%u, expected 3/2\n"
				, c_fn, c_call);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** both compound assignments must reach memory. */
	if(c_store < 2) {
		fprintf(stderr, "[assign-compound-addr] stores: %u, expected >= 2\n"
				, c_store);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[assign-compound-addr] cannot write %s\n"
					, ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
