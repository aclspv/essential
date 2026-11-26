#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>
#include <ae2f/Keys.h>

static const char s_src[] =
"typedef int i32;\n"
"inline i32 kwd(i32 x) { return x + 1; }\n"
"__attribute__((always_inline)) i32 aif(i32 x) { i32 y=x; if(x>0){y=x+1;} return y; }\n"
"__attribute__((noinline)) i32 ni(i32 x) { return x + 2; }\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void entry(__attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out)\n"
"{ *out = kwd(1) + ni(2) + aif(3); }\n";

static int s_run(aclspv_wrd_t c_flag, unsigned c_want_call)
{
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.c"};
	aclspv_compile_t RET;
	aclspv_wrd_t* spirv = 0;
	aclspv_wrdcount_t count = 0;
	unsigned calls;
	acltest_spirv_stats stats;

	acltestcfg();
	files[0].Contents = s_src; files[0].Filename = "main.c"; files[0].Length = sizeof(s_src)-1;
	files[1].Contents = s_oclbase; files[1].Filename = "opencl-c-base.h"; files[1].Length = sizeof(s_oclbase)-1;
	RET = dbg_aclspv_compile(files, 2, ARG, 1, &count, &spirv, 0, c_flag);
	if(RET != ACLSPV_COMPILE_OK || !spirv) { fprintf(stderr, "compile failed %d\n", (int)RET); free(spirv); return 1; }
	if(acltest_spirv_scan(spirv, count, &stats)) { fputs("bad spir-v\n", stderr); free(spirv); return 1; }
	calls = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionCall);
	printf("flag=%u calls=%u\n", (unsigned)c_flag, calls);
	if(calls != c_want_call) { fputs("unexpected OpFunctionCall count\n", stderr); free(spirv); return 1; }
	{ FILE* fp = fopen(ACLTEST_OUT, "wb"); if(fp) { fwrite(spirv, sizeof(*spirv), (size_t)count, fp); fclose(fp); } }
	free(spirv);
	return 0;
}

int main(void)
{
	if(s_run(0, 2)) return EXIT_FAILURE;
	if(s_run(ACLSPV_CTX_FLAG0_INLINE_BY_DEFAULT, 1)) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
