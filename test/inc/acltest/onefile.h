#ifndef	acltest_onefile_h
#define acltest_onefile_h

#include <.acltest/ocl.h>
#include <.acltest/source.h>

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
];

static char s_oclbase[
	sizeof(OCLBASE_0)
	+ sizeof(OCLBASE_1) - 1 
	+ sizeof(OCLBASE_2) - 1
	+ sizeof(OCLBASE_3) - 1
];

#include <string.h>

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

	strcpy(s_oclbase, OCLBASE_0);
	strcat(s_oclbase, OCLBASE_1);
	strcat(s_oclbase, OCLBASE_2);
	strcat(s_oclbase, OCLBASE_3);
}

#endif
