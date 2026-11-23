#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** fp8 literal casts.
 *
 * an fp8 literal is carried as its 8-bit encoding. the float and the integer
 * sources must both fold to the OCP "FP8 for Deep Learning" bit patterns:
 *   E4M3 0.5 = 0x30, 1.0 = 0x38, 3.0 = 0x44
 *   E5M2 0.5 = 0x38, 1.0 = 0x3C
 * */
static const char s_src_fp8_literal[] =
"typedef __attribute__((annotate(\"__fp8_e4m3\"))) __float128 f8a;\n"
"typedef __attribute__((annotate(\"__fp8_e5m2\"))) __float128 f8b;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f8a* outa,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f8b* outb\n"
") {\n"
"    *outa = (f8a)0.5f;\n"
"    *outb = (f8b)1.0f;\n"
"    *outa = (f8a)3;\n"
"    *outb = (f8b)0.5f;\n"
"}\n";

enum {
	OP_TYPE_FLOAT	= 22,
	OP_CONSTANT	= 43
};

/** id of the 8-bit OpTypeFloat with encoding `c_encoding`, or 0. */
static aclspv_wrd_t s_f8_type(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_encoding
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_TYPE_FLOAT
				&& wc >= 4
				&& rd[i + 2] == 8
				&& rd[i + 3] == c_encoding)
			return rd[i + 1];

		i += wc;
	}

	return 0;
}

/** 1 when an OpConstant of type `c_ty` carries `c_bits` in its low byte. */
static int s_has_const(
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
				&& (rd[i + 3] & 0xFFu) == c_bits)
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
	aclspv_wrd_t		TY4;
	aclspv_wrd_t		TY5;

	acltestcfg();

	files[0].Contents	= s_src_fp8_literal;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_fp8_literal) - 1;

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
		fprintf(stderr, "[fp8-literal] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[fp8-literal] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[fp8-literal] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	TY4 = s_f8_type(spirv, count, (aclspv_wrd_t)SpvFPEncodingFloat8E4M3EXT);
	TY5 = s_f8_type(spirv, count, (aclspv_wrd_t)SpvFPEncodingFloat8E5M2EXT);

	if(!TY4 || !TY5) {
		fputs("[fp8-literal] missing fp8 type(s)\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_const(spirv, count, TY4, 0x30u)) {
		fputs("[fp8-literal] (f8a)0.5f constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_const(spirv, count, TY4, 0x44u)) {
		fputs("[fp8-literal] (f8a)3 constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_const(spirv, count, TY5, 0x3Cu)) {
		fputs("[fp8-literal] (f8b)1.0f constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_const(spirv, count, TY5, 0x38u)) {
		fputs("[fp8-literal] (f8b)0.5f constant is wrong\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[fp8-literal] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
