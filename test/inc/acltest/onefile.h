#ifndef	acltest_onefile_h
#define acltest_onefile_h

/** path the test writes its SPIR-V module to.
 *
 * the build passes -DACLTEST_OUT="<path>" per test target (see
 * cmake/test.cmake); the fallback keeps a standalone invocation writing
 * into its working directory.
 * */
#ifndef ACLTEST_OUT
#define ACLTEST_OUT "result.spv"
#endif

#include <.acltest/ocl.h>
#include <.acltest/source.h>
#include <ae2f/cc/inline.h>

static char s_src0[
	sizeof(SRC0_0)
	+ sizeof(SRC0_1) - 1
	+ sizeof(SRC0_2) - 1
	+ sizeof(SRC0_3) - 1
	+ sizeof(SRC0_4) - 1
	+ sizeof(SRC0_5) - 1
	+ sizeof(SRC0_6) - 1
	+ sizeof(SRC0_7) - 1
	+ sizeof(SRC0_8) - 1
	+ sizeof(SRC0_9) - 1
	+ sizeof(SRC0_10) - 1
	+ sizeof(SRC0_11) - 1
	+ sizeof(SRC0_12) - 1
	+ sizeof(SRC0_13) - 1
	+ sizeof(SRC0_14) - 1
	+ sizeof(SRC0_15) - 1
	+ sizeof(SRC0_16) - 1
	+ sizeof(SRC0_17) - 1
	+ sizeof(SRC0_18) - 1
	+ sizeof(SRC0_19) - 1
	+ sizeof(SRC0_20) - 1
	+ sizeof(SRC0_21) - 1
	+ sizeof(SRC0_22) - 1
	+ sizeof(SRC0_23) - 1
	+ sizeof(SRC0_24) - 1
	+ sizeof(SRC0_25) - 1
];

static char s_oclbase[
	sizeof(OCLBASE_0)
	+ sizeof(OCLBASE_1) - 1
	+ sizeof(OCLBASE_2) - 1
	+ sizeof(OCLBASE_3) - 1
];

#include <string.h>

/** concatenate short literals into `rd_dst`.
 *
 * strict C90 only guarantees 509 characters per string literal, so shader
 * sources longer than that are assembled from an array of shorter lines at
 * runtime instead of one oversized literal.
 * */
ae2f_inline_f static void acltest_concat(
		char* ae2f_restrict	rd_dst,
		const char* const*	rd_lines,
		unsigned		c_num
		) {
	unsigned i;
	rd_dst[0] = '\0';
	for(i = 0; i < c_num; ++i)
		strcat(rd_dst, rd_lines[i]);
}

static void acltestcfg(void) {
	strcpy(s_src0, SRC0_0);
	strcat(s_src0, SRC0_1);
	strcat(s_src0, SRC0_2);
	strcat(s_src0, SRC0_3);
	strcat(s_src0, SRC0_4);
	strcat(s_src0, SRC0_5);
	strcat(s_src0, SRC0_6);
	strcat(s_src0, SRC0_7);
	strcat(s_src0, SRC0_8);
	strcat(s_src0, SRC0_9);
	strcat(s_src0, SRC0_10);
	strcat(s_src0, SRC0_11);
	strcat(s_src0, SRC0_12);
	strcat(s_src0, SRC0_13);
	strcat(s_src0, SRC0_14);
	strcat(s_src0, SRC0_15);
	strcat(s_src0, SRC0_16);
	strcat(s_src0, SRC0_17);
	strcat(s_src0, SRC0_18);
	strcat(s_src0, SRC0_19);
	strcat(s_src0, SRC0_20);
	strcat(s_src0, SRC0_21);
	strcat(s_src0, SRC0_22);
	strcat(s_src0, SRC0_23);
	strcat(s_src0, SRC0_24);
	strcat(s_src0, SRC0_25);

	strcpy(s_oclbase, OCLBASE_0);
	strcat(s_oclbase, OCLBASE_1);
	strcat(s_oclbase, OCLBASE_2);
	strcat(s_oclbase, OCLBASE_3);
}

#endif
