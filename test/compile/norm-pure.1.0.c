#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * the purity half of the `OpFunction` control mask.
 *
 * add3 reads and writes nothing it does not own: the analysed body is both
 * Const and Pure. ext has no body, so its declared `pure` is what the
 * declaration can carry: Pure only. the entry point carries neither.
 * */
static const char s_src_norm_pure[] =
"typedef int i32;\n"
"i32 add3(i32 a) { return a + 3; }\n"
"__attribute__((pure)) i32 ext(i32 x);\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_pure(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = add3(1) + ext(2);\n"
"}\n";

struct s_ctrl {
	unsigned	m_fn;
	unsigned	m_const;
	unsigned	m_pure;
};

/** count OpFunction by purity bits: [header][ret][id][control][type]. */
static int s_count_ctrl(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		struct s_ctrl*		wr
		) {
	aclspv_wrdcount_t i = 5;

	wr->m_fn	= 0;
	wr->m_const	= 0;
	wr->m_pure	= 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 1;

		if(op == (unsigned)SpvOpFunction && wc >= 5) {
			const aclspv_wrd_t ctrl = rd[i + 3];

			++wr->m_fn;
			if(ctrl & (aclspv_wrd_t)SpvFunctionControlConstMask)
				++wr->m_const;
			if(ctrl & (aclspv_wrd_t)SpvFunctionControlPureMask)
				++wr->m_pure;
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

	acltestcfg();

	files[0].Contents	= s_src_norm_pure;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_norm_pure) - 1;

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
		fprintf(stderr, "[norm-pure] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[norm-pure] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-pure] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(s_count_ctrl(spirv, count, &ctrl)) {
		fputs("[norm-pure] malformed OpFunction\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("fn=%u const=%u pure=%u\n", ctrl.m_fn, ctrl.m_const, ctrl.m_pure);

	/** add3, the ext declaration, and the entry point. */
	if(ctrl.m_fn != 3 || ctrl.m_const != 1 || ctrl.m_pure != 2) {
		fputs("[norm-pure] function control mismatch\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-pure] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
