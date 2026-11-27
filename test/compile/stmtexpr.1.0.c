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
 * GNU statement expressions: `({ stmt; stmt; expr; })` used as an operand.
 *
 *  - a block-local declaration, a side effect and a value
 *  - a nested statement expression (inside another one)
 *  - control flow inside the block, with the tail expression after the merge
 *
 * the value of each block must reach its store, so the module must contain
 * the arithmetic and the store, the `if` must have carved a real branch, and
 * the module must validate.
 * */
static const char s_src_stmtexpr[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    i32 n = ({ i32 t = *in; t + 1; });\n"
"    *out = ({ *out = n; n * 2; });\n"
"    *out = ({ i32 a = ({ *in + 3; }); i32 b = ({ a * 5; }); a + b; });\n"
"    *out = ({ i32 r = *in; if (r < 0) { r = 0 - r; } r; });\n"
"    *out = ({ 7; });\n"
"}\n";

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;

	acltestcfg();

	files[0].Contents	= s_src_stmtexpr;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_stmtexpr) - 1;

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
		fprintf(stderr, "[stmtexpr] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[stmtexpr] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[stmtexpr] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("inst=%u store=%u add=%u mul=%u brc=%u selmerge=%u targets_ok=%d\n"
			, stats.m_num_inst
			, stats.m_num_store
			, stats.m_num_iadd
			, stats.m_num_imul
			, stats.m_num_brc
			, stats.m_num_selmerge
			, stats.m_targets_ok
			);

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[stmtexpr] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	if(stats.m_num_store < 5
			|| stats.m_num_iadd < 3
			|| stats.m_num_imul < 2
			|| stats.m_num_brc < 1
			|| stats.m_num_selmerge < 1
			|| !stats.m_targets_ok) {
		fputs("[stmtexpr] statement expression value or control flow missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	free(spirv);
	return EXIT_SUCCESS;
}
