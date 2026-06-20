#include <aclspv.hh>
#include <assert.h>
#include <fstream>
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
	aclspv::ctx CTX(0,
				    ACLSPV_CTX_FLAG0_SYMBOL
				    | ACLSPV_CTX_FLAG0_OGL
				    | ACLSPV_CTX_FLAG0_LOGICAL
				    | ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS
			, aclspv::ver_spv(1, 0) /** spir-v 1.0 */
	);

	aclspv_wrd_t *spirv0;
	aclspv_wrdcount_t spirv_count;

	acltestcfg();

	printf("flag0: %u\n", static_cast<unsigned>(CTX.me().m_flag0));

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
