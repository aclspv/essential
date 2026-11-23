#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** bf16 literal casts.
 *
 * a bf16 literal is carried as its binary16 pattern: the truncated high half
 * of the binary32 image. both the float and the integer source lane must fold
 * to the bf16 encoding, not the IEEE binary16 one.
 * */
static const char s_src_bf16_literal[] =
"typedef __attribute__((annotate(\"__bf16\"))) __float128 bf16;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) bf16* out\n"
") {\n"
"    *out = (bf16)0.5f;\n"
"    *out = (bf16)1.0f;\n"
"    *out = (bf16)3;\n"
"}\n";

enum {
	OP_TYPE_FLOAT	= 22,
	OP_CONSTANT	= 43
};

/** id of the 16-bit OpTypeFloat with the BFloat16KHR encoding, or 0. */
static aclspv_wrd_t s_bf16_type(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_TYPE_FLOAT
				&& wc >= 4
				&& rd[i + 2] == 16
				&& rd[i + 3] == (aclspv_wrd_t)SpvFPEncodingBFloat16KHR)
			return rd[i + 1];

		i += wc;
	}

	return 0;
}

/** 1 when an OpConstant of type `c_ty` carries `c_bits` in its low half. */
static int s_has_bf16_const(
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

	files[0].Contents	= s_src_bf16_literal;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_bf16_literal) - 1;

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
		fprintf(stderr, "[bf16-literal] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[bf16-literal] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[bf16-literal] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	TY = s_bf16_type(spirv, count);

	if(!TY) {
		fputs("[bf16-literal] no 16-bit bf16 type\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** 0.5f, 1.0f and 3.0f as bf16. */
	if(!s_has_bf16_const(spirv, count, TY, 0x3F00u)) {
		fputs("[bf16-literal] (bf16)0.5f constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_bf16_const(spirv, count, TY, 0x3F80u)) {
		fputs("[bf16-literal] (bf16)1.0f constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_bf16_const(spirv, count, TY, 0x4040u)) {
		fputs("[bf16-literal] (bf16)3 constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[bf16-literal] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
