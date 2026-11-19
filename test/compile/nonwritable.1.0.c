#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * `const` on a storage-buffer argument's pointee marks the variable
 * `NonWritable`.
 *
 * in_a is const: a vertex/geometry pipeline may use it without
 * vertexPipelineStoresAndAtomics. in_b is not: it stays writable. exactly
 * one NonWritable decoration is emitted.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_nw(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(12)\")))\n",
	"    const i32 in_a[],\n",
	"    __attribute__((annotate(\"aclspv_storage_class(12)\")))\n",
	"    i32 in_b[],\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n",
	") {\n",
	"    *out = in_a[0] + in_b[0];\n",
	"}\n",
};

/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

static unsigned s_count_nonwritable(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c
		) {
	aclspv_wrdcount_t i = 5;
	unsigned n = 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			break;

		if((unsigned)(rd[i] & 0xFFFFu) == (unsigned)SpvOpDecorate
				&& wc >= 3
				&& (unsigned)(rd[i + 2] & 0xFFFFu)
					== (unsigned)SpvDecorationNonWritable)
			++n;

		i += wc;
	}

	return n;
}

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		num_nw;

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
		fprintf(stderr, "[nonwritable.1.0] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[nonwritable.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	num_nw = s_count_nonwritable(spirv, count);

	printf("nonwritable=%u\n", num_nw);

	if(num_nw != 1) {
		fprintf(stderr, "[nonwritable.1.0] NonWritable decorations: %u, expected 1\n"
				, num_nw);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[nonwritable.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
