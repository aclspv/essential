#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** f16 arithmetic is native.
 *
 * with the Float16 capability present, `c + c` (both f16) is a direct
 * `OpFAdd` on the f16 type -- it must not be widened to f32.
 * */
static const char s_src_f16_arith[] =
"typedef _Float16 f16;\n"
"f16 a(f16 c) {\n"
"\treturn c + c;\n"
"}\n";

enum {
	OP_TYPE_FLOAT	= 22
};

/** id of the native 16-bit OpTypeFloat (f16), or 0. */
static aclspv_wrd_t s_f16_type(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_TYPE_FLOAT && wc == 3 && rd[i + 2] == 16)
			return rd[i + 1];

		i += wc;
	}

	return 0;
}

/** 1 when an instruction `c_op` exists whose result type is `c_ty`. */
static int s_has_op_ty(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		unsigned		c_op,
		aclspv_wrd_t		c_ty
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == c_op && wc >= 3 && rd[i + 1] == c_ty)
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
	aclspv_wrd_t		TY_F16;

	acltestcfg();

	files[0].Contents	= s_src_f16_arith;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_f16_arith) - 1;

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
		fprintf(stderr, "[f16-arith] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[f16-arith] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[f16-arith] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	TY_F16 = s_f16_type(spirv, count);

	if(!TY_F16) {
		fputs("[f16-arith] missing f16 type\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_op_ty(spirv, count, (unsigned)SpvOpFAdd, TY_F16)) {
		fputs("[f16-arith] no native OpFAdd on the f16 type\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[f16-arith] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
