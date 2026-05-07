#include <aclspv.hh>
#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./source.auto.h"
#include "./ocl.auto.h"
#include "aclspv.h"
char maincontent[sizeof(A_0) + sizeof(A_1) - 1 + sizeof(A_2) - 1
#if 0
	+ sizeof(A_3) - 1
	+ sizeof(A_4) - 1
	+ sizeof(A_5) - 1
	+ sizeof(A_6) - 1
#endif
];

char oclhead[sizeof(B_0) + sizeof(B_1) - 1 + sizeof(B_2) - 1];
#if 1
int main(void) {
	struct CXUnsavedFile files[2];
	const char *ARG[] = {"main.cl"};
	enum ACLSPV_COMPILE_ RET;
	aclspv::ctx CTX(10,	
			    aclspv::ctx_flag0(
				    ACLSPV_CTX_FLAG0_SYMBOL
				    , ACLSPV_CTX_FLAG0_OGL
				    , ACLSPV_CTX_FLAG0_LOGICAL
				    , ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS)
			, aclspv::ver_spv(1, 0) /** spir-v 1.2 */
	);

	aclspv_wrd_t *spirv0;
	aclspv_wrdcount_t spirv_count;

	strcpy(maincontent, A_0);
	strcat(maincontent, A_1);
	strcat(maincontent, A_2);
#if 0
		strcat(maincontent, A_3);
		strcat(maincontent, A_4);
		strcat(maincontent, A_5);
		strcat(maincontent, A_6);
#endif

	strcpy(oclhead, B_0);
	strcat(oclhead, B_1);
	strcat(oclhead, B_2);

	files[0].Contents = maincontent;
	files[0].Filename = "main.cl";
	files[0].Length = sizeof(maincontent) - 1;

	puts("main.cl::::::::::::::::::");
	puts(files[0].Contents);
	puts("main.cl::::::::::::::::::");

	files[1].Contents = oclhead;
	files[1].Filename = "./opencl-c-base.h";
	files[1].Length = sizeof(oclhead) - 1;

	puts("opencl-c-base.h::::::::::::::::::");
	puts(files[1].Contents);
	puts("opencl-c-base.h::::::::::::::::::");

	RET = CTX.compile3(files, 2, ARG, 1, 0);
	ae2f_ifnezerr(RET) printf("[err] aclspv_compile:	%d\n", RET);
	RET = CTX.dump_output(spirv_count, spirv0);
	ae2f_ifnezerr(RET) printf("[err] dump_output:	%d\n", RET);

	printf("output spirv: %p, %u\n", reinterpret_cast<void *>(spirv0),
	       spirv_count);

	if (spirv0) {
		std::ofstream ofs("result-cc.spv", std::ios::binary);
		unless(ofs.is_open()) goto FPOPEN_FAILED;
		ofs.write(reinterpret_cast<char *>(spirv0),
			  static_cast<std::streamsize>(
			      static_cast<size_t>(spirv_count) *
			      sizeof(aclspv::wrd_t)));
		ofs.close();

	FPOPEN_FAILED:;
	}

	puts("freeing spirv");
	free(spirv0);

	return 0;
}
#else

#endif
