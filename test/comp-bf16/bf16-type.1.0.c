#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/** bfloat16 as a value type.
 *
 * bf16 is spelled as `__float128` carrying an `annotate("__bf16")`. the type
 * layer must emit a 16-bit OpTypeFloat with the BFloat16KHR floating-point
 * encoding, declare the BFloat16TypeKHR capability and the SPV_KHR_bfloat16
 * extension, and lower the float -> bf16 conversion to OpFConvert.
 * */
static const char s_src_bf16_type[] =
"typedef __attribute__((annotate(\"__bf16\"))) __float128  bf16;\n"
"typedef float f32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f32* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) bf16* out\n"
") {\n"
"    *out = (bf16)(*in);\n"
"}\n";

enum {
	OP_EXTENSION	= 10,
	OP_CAPABILITY	= 17,
	OP_TYPE_FLOAT	= 22
};

/** 1 when the module declares `c_ext`. */
static int s_has_extension(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		const char*		c_ext
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_EXTENSION) {
			const char* const STR = (const char*)&rd[i + 1];
			const aclspv_wrdcount_t MAX = (wc - 1u) * 4u;
			aclspv_wrdcount_t j = 0;

			while(j < MAX && c_ext[j] && STR[j] == c_ext[j])
				++j;

			if(j < MAX && !c_ext[j] && !STR[j])
				return 1;
		}

		i += wc;
	}

	return 0;
}

/** 1 when an OpTypeFloat has width 16 and the BFloat16KHR encoding. */
static int s_has_bf16_type(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t*		r_ty
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)OP_TYPE_FLOAT
				&& wc >= 4
				&& rd[i + 2] == 16
				&& rd[i + 3] == (aclspv_wrd_t)SpvFPEncodingBFloat16KHR) {
			if(r_ty) *r_ty = rd[i + 1];
			return 1;
		}

		i += wc;
	}

	return 0;
}

static int s_has_op(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		unsigned		c_op
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == c_op)
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

	acltestcfg();

	files[0].Contents	= s_src_bf16_type;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_bf16_type) - 1;

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
		fprintf(stderr, "[bf16-type] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[bf16-type] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[bf16-type] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_bf16_type(spirv, count, 0)) {
		fputs("[bf16-type] no 16-bit OpTypeFloat with BFloat16KHR encoding\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!s_has_extension(spirv, count, "SPV_KHR_bfloat16")) {
		fputs("[bf16-type] SPV_KHR_bfloat16 extension is not declared\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		int found = 0;
		aclspv_wrdcount_t i = 5;

		while(i < count) {
			const unsigned wc = (unsigned)(spirv[i] >> 16);
			const unsigned op = (unsigned)(spirv[i] & 0xffff);

			if(!wc || i + wc > count)
				break;

			if(op == (unsigned)OP_CAPABILITY
					&& spirv[i + 1] == (aclspv_wrd_t)SpvCapabilityBFloat16TypeKHR)
				found = 1;

			i += wc;
		}

		if(!found) {
			fputs("[bf16-type] BFloat16TypeKHR capability is not declared\n", stderr);
			free(spirv);
			return EXIT_FAILURE;
		}
	}

	if(!s_has_op(spirv, count, (unsigned)SpvOpFConvert)) {
		fputs("[bf16-type] float -> bf16 conversion is not an OpFConvert\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[bf16-type] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
