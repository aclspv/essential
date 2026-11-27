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
 * a pointer store whose value is a pointer load, followed by a store to the
 * loaded pointer.
 *
 *   tmp = *a;  *a = *b;  *b = tmp;
 *
 * `*b` loads into an id that the `*a` store consumes; `*b = tmp` then
 * replaces b's tracked value. the load must stay emitted: it used to be
 * dropped by the reference gate when the tracked value was released, leaving
 * `OpStore %a %<load>` naming an undefined id.
 *
 * counts: `swap` and the entry point (2 OpFunction), three loads (two in
 * swap, one for `a` in the entry point).
 * */
static const char s_src_norm_swap[] =
"typedef int i32;\n"
"void swap(i32* a, i32* b) {\n"
"    i32 tmp = *a;\n"
"    *a = *b;\n"
"    *b = tmp;\n"
"}\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_swap(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    i32 a = 10;\n"
"    i32 b = 20;\n"
"    swap(&a, &b);\n"
"    *out = a;\n"
"}\n";

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_fn;
	unsigned		c_load;

	acltestcfg();

	files[0].Contents	= s_src_norm_swap;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_norm_swap) - 1;

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
		fprintf(stderr, "[norm-swap] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[norm-swap] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-swap] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_fn	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);
	c_load	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpLoad);

	printf("fn=%u load=%u\n", c_fn, c_load);

	if(c_fn != 2) {
		fprintf(stderr, "[norm-swap] functions: %u, expected 2\n", c_fn);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** `*a` and `*b` are both consumed by stores: neither may be elided. */
	if(c_load != 3) {
		fprintf(stderr, "[norm-swap] loads: %u, expected 3\n", c_load);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-swap] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
