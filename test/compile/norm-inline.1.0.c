#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * `OpFunction` function control, read from the declaration when the function
 * is emitted.
 *
 * inl:   `always_inline` -> Inline
 * never: `noinline`      -> DontInline
 * plain, entry point     -> None
 * */
static const char s_src_norm_inline[] =
"typedef int i32;\n"
"__attribute__((always_inline)) i32 inl(i32 x) { return x + 1; }\n"
"__attribute__((noinline)) i32 never(i32 x) { return x + 2; }\n"
"i32 plain(i32 x) { return x + 3; }\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_ctrl(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = inl(1) + never(2) + plain(3);\n"
"}\n";

struct s_ctrl {
	unsigned	m_inline;
	unsigned	m_dont;
	unsigned	m_none;
};

/** count OpFunction by control mask: [header][ret][id][control][type]. */
static int s_count_ctrl(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		struct s_ctrl*		wr
		) {
	aclspv_wrdcount_t i = 5;

	wr->m_inline	= 0;
	wr->m_dont	= 0;
	wr->m_none	= 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 1;

		if(op == (unsigned)SpvOpFunction && wc >= 5) {
			const aclspv_wrd_t ctrl = rd[i + 3];

			if(ctrl & (aclspv_wrd_t)SpvFunctionControlInlineMask)
				++wr->m_inline;
			else if(ctrl & (aclspv_wrd_t)SpvFunctionControlDontInlineMask)
				++wr->m_dont;
			else
				++wr->m_none;
		}

		i += wc;
	}

	return 0;
}

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	struct s_ctrl		ctrl;
	unsigned		c_fn;

	acltestcfg();

	files[0].Contents	= s_src_norm_inline;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_norm_inline) - 1;

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
		fprintf(stderr, "[norm-inline] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[norm-inline] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-inline] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_fn = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);

	if(s_count_ctrl(spirv, count, &ctrl)) {
		fputs("[norm-inline] malformed OpFunction\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("fn=%u inline=%u dontinline=%u none=%u\n"
			, c_fn, ctrl.m_inline, ctrl.m_dont, ctrl.m_none);

	if(c_fn != 4 || ctrl.m_inline != 1
			|| ctrl.m_dont != 1 || ctrl.m_none != 2) {
		fputs("[norm-inline] function control mismatch\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-inline] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
