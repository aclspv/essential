#include <aclspv.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acltest/onefile.h>
#include <acltest/compile.h>

#if 1
int main(void);
int main(void) {
	struct CXUnsavedFile files[2];
	const char *ARG[] = {"main.cl"};
	enum ACLSPV_COMPILE_ RET;

	aclspv_wrd_t *spirv0;
	aclspv_wrdcount_t spirv_count;

	acltestcfg();

	files[0].Contents = s_src0;
	files[0].Filename = "main.cl";
	files[0].Length = sizeof(s_src0) - 1;

	puts("main.cl::::::::::::::::::");
	puts(files[0].Contents);
	puts("main.cl::::::::::::::::::");

	files[1].Contents = s_oclbase;
	files[1].Filename = "./opencl-c-base.h";
	files[1].Length = sizeof(s_oclbase) - 1;

	puts("opencl-c-base.h::::::::::::::::::");
	puts(files[1].Contents);
	puts("opencl-c-base.h::::::::::::::::::");

	RET = dbg_aclspv_compile(
			files
			, 2
			, ARG
			, 1
			, &spirv_count
			, &spirv0
			, ae2f_NIL
			);

	printf("aclspv_compile: %d\n", (int) RET);
	printf("output spirv: %p, %u\n", (void *)spirv0, spirv_count);

	assert(spirv0);

	if (spirv0) {
		FILE *const fp = fopen("result.spv", "wb");
		unless(fp) goto FPOPEN_FAILED;

		fwrite(spirv0, sizeof(aclspv_wrd_t), (size_t)spirv_count, fp);

	FPOPEN_FAILED:
		if(fp) fclose(fp);
	}

	puts("freeing spirv");
	free(spirv0);

	return 0;
}
#else

#endif
