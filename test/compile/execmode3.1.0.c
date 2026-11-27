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
 * `aclspv_execmode3(mode, #a, #b, #c)` with literal operands.
 *
 * the spelling is parsed and emitted as one OpExecutionMode:
 * [op][entry][mode][a][b][c]. this is the literal form, valid at SPIR-V 1.0;
 * the spec-id form needs 1.2 and is rejected earlier.
 * */
static const char s_src_em3[] =
"__attribute__((annotate(\"aclspv_execmode3(17, #4, #2, #1)\")))\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"void t_em3(void) { return; }\n";

/** the single OpExecutionMode carries the parsed mode and operands. */
static int s_check_mode(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	aclspv_wrdcount_t i = 5;
	unsigned n = 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 1;

		if(op == (unsigned)SpvOpExecutionMode) {
			++n;
			if(wc != 6 || rd[i + 2] != 17
					|| rd[i + 3] != 4
					|| rd[i + 4] != 2
					|| rd[i + 5] != 1)
				return 1;
		}

		i += wc;
	}

	return n == 1 ? 0 : 1;
}

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;

	acltestcfg();

	files[0].Contents	= s_src_em3;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_em3) - 1;

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

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fprintf(stderr, "[execmode3.1.0] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[execmode3.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[execmode3.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	if(s_check_mode(spirv, count)) {
		fputs("[execmode3.1.0] OpExecutionMode does not match the annotation\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	free(spirv);
	return EXIT_SUCCESS;
}
