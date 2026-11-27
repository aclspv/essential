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
 * normal (non-entry) function bodies.
 *
 * two functions are defined and never called: an expression body with
 * parameters, and a control-flow body with two returns. the emitter must
 * produce OpFunction + OpFunctionParameter + OpReturnValue for them, and the
 * module must still validate next to the entry point.
 * */
static const char s_src_norm_fn[] =
"typedef int i32;\n"
"int add3(i32 a, i32 b, i32 c) { return a + b + c; }\n"
"int pick(i32 x) {\n"
"    if (x) { return 1; }\n"
"    return 0;\n"
"}\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_norm(void) { }\n";

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_fn;
	unsigned		c_arg;
	unsigned		c_retval;

	acltestcfg();

	files[0].Contents	= s_src_norm_fn;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_norm_fn) - 1;

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
			, 0
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[norm-fn] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[norm-fn] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-fn] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_fn = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);
	c_arg = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunctionParameter);
	c_retval = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpReturnValue);

	printf("fn=%u arg=%u retval=%u lbl=%u\n"
			, c_fn, c_arg, c_retval, stats.m_num_lbl);

	/** add3, pick, and the entry point. */
	if(c_fn != 3) {
		fprintf(stderr, "[norm-fn] functions: %u, expected 3\n", c_fn);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** add3 contributes three parameters, pick one, the entry none. */
	if(c_arg != 4) {
		fprintf(stderr, "[norm-fn] parameters: %u, expected 4\n", c_arg);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** add3 returns once, pick twice. */
	if(c_retval < 3) {
		fprintf(stderr, "[norm-fn] return values: %u, expected at least 3\n"
				, c_retval);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-fn] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
