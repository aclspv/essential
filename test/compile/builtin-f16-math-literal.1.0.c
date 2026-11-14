#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** scalar f16 literal folding for the whole family.
 *
 * an f16 literal is carried as its binary16 bit pattern. the host fold takes
 * the value, applies sinf/cosf/tanf and truncates back to binary16, so no
 * OpExtInst may appear. the expected constants below are the truncated
 * binary16 images of sin/cos/tan(0.5f).
 * */
static const char s_src_f16_math_literal[] =
"typedef _Float16 f16;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f16* out\n"
") {\n"
"    const f16 half = 1.0f;\n"
"    *out = __builtin_sinf16(0.5f);\n"
"    *out = __builtin_cosf16(0.5f);\n"
"    *out = __builtin_tanf16(0.5f);\n"
"    *out = __builtin_sinf16(half);\n"
"}\n";

enum {
	OP_TYPE_FLOAT	= 22,
	OP_CONSTANT	= 43
};

/** id of the 16-bit OpTypeFloat, or 0. */
static aclspv_wrd_t s_f16_type(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_TYPE_FLOAT && wc >= 3 && rd[i + 2] == 16)
			return rd[i + 1];

		i += wc;
	}

	return 0;
}

/** 1 when an OpConstant of type `c_ty` carries `c_bits` in its low half. */
static int s_has_f16_const(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_ty,
		aclspv_wrd_t		c_bits
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_CONSTANT
				&& wc >= 4
				&& rd[i + 1] == c_ty
				&& (rd[i + 3] & 0xFFFFu) == c_bits)
			return 1;

		i += wc;
	}

	return 0;
}

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	aclspv_wrd_t		TY;

	acltestcfg();

	files[0].Contents	= s_src_f16_math_literal;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_f16_math_literal) - 1;

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
		fprintf(stderr, "[f16-math-literal] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[f16-math-literal] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[f16-math-literal] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u extinst=%u sin=%u cos=%u tan=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_extinst
			, stats.m_num_sin
			, stats.m_num_cos
			, stats.m_num_tan
			, stats.m_targets_ok
			);

	if(stats.m_num_extinst
			|| stats.m_num_sin || stats.m_num_cos || stats.m_num_tan
			|| !stats.m_targets_ok) {
		fputs("[f16-math-literal] literal math was not folded\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	TY = s_f16_type(spirv, count);

	if(!TY) {
		fputs("[f16-math-literal] no 16-bit float type\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_f16_const(spirv, count, TY, 0x37ABu)) {
		fputs("[f16-math-literal] sinf16 constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_f16_const(spirv, count, TY, 0x3B05u)) {
		fputs("[f16-math-literal] cosf16 constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_f16_const(spirv, count, TY, 0x385Eu)) {
		fputs("[f16-math-literal] tanf16 constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the const f16 variable is loaded as its bit pattern, so the fold must
	 * decode binary16 before applying sinf. */
	if(!s_has_f16_const(spirv, count, TY, 0x3ABBu)) {
		fputs("[f16-math-literal] const half variable was not folded\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[f16-math-literal] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
