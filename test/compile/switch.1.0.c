/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * a switch with:
 *  - stacked empty labels (case 1: case 2:)
 *  - fall-through from case 3 into case 4
 *  - a default in the middle
 *  - a break in every arm
 *  - a second switch without a default (its absent-default edge is the
 *    selection merge target)
 * and a value that is assigned on every arm and read after the switch, so
 * the merge needs OpPhi entries for each arm.
 *
 * the test targets are compiled with -DNDEBUG, so every check is an inline
 * branch returning failure.
 * */
static const char* const s_src_switch_lines[] = {
"typedef int i32;\n",
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
"void t_switch(\n",
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in_n,\n",
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in_k,\n",
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_r\n",
") {\n",
"    i32 n = *in_n;\n",
"    i32 k = *in_k;\n",
"    i32 r = 0;\n",
"    i32 j = 0;\n",
"    switch (k) {\n",
"    case 0:\n",
"        r = n;\n",
"        break;\n",
"    case 1:\n",
"    case 2:\n",
"        r = n + 1;\n",
"        break;\n",
"    case 3:\n",
"        r = n + 2;\n",
"    case 4:\n",
"        r = r + 3;\n",
"        break;\n",
"    default:\n",
"        r = 0 - 1;\n",
"        break;\n",
"    }\n",
"    switch (n) {\n",
"    case 5:\n",
"        r = r + 5;\n",
"    }\n",
"    while (j < n) {\n",
"        switch (j) {\n",
"        case 6:\n",
"            j = j + 1;\n",
"            continue;\n",
"        case 7:\n",
"            break;\n",
"        default:\n",
"            j = j + 1;\n",
"            break;\n",
"        }\n",
"        j = j + 1;\n",
"    }\n",
"    r = r + j;\n",
"    *out_r = r;\n",
"}\n"
};

static char s_src_switch[2048];

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;

	acltestcfg();

	acltest_concat(s_src_switch, s_src_switch_lines,
			(unsigned)(sizeof(s_src_switch_lines) / sizeof(s_src_switch_lines[0])));

	files[0].Contents	= s_src_switch;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_switch) - 1;

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
		fprintf(stderr, "[switch.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[switch.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[switch.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u lbl=%u br=%u brc=%u sw=%u selmerge=%u phi=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_lbl
			, stats.m_num_br
			, stats.m_num_brc
			, stats.m_num_switch
			, stats.m_num_selmerge
			, stats.m_num_phi
			, stats.m_targets_ok
			);

	if(!stats.m_num_lbl
			|| stats.m_num_switch < 2
			|| stats.m_num_selmerge < 2
			|| stats.m_num_br < 1
			|| stats.m_num_phi < 1
			|| !stats.m_targets_ok) {
		fputs("[switch.1.0] switch control flow missing or targets invalid\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[switch.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
