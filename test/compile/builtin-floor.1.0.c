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

/** `__builtin_floorf` -> GLSL.std.450 Floor on f32. */
static const char s_src_builtin_floorf[] =
"typedef int i32;\n"
"typedef float f32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) i32* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32* out\n"
") {\n"
"    *out = __builtin_floorf((f32)(*in));\n"
"}\n";

/** `__builtin_elementwise_floor` -> GLSL.std.450 Floor on a vector. */
static const char s_src_builtin_elementwise_floor[] =
"typedef int i32;\n"
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1,1,1)\")))\n"
"void t(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\"))) f32x4* in,\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) f32x4* out\n"
") {\n"
"    *out = __builtin_elementwise_floor(*in);\n"
"}\n";

int main(void);

static int s_run(const char* c_src, const char* c_tag) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;

	acltestcfg();

	files[0].Contents	= c_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned)strlen(c_src);

	files[1].Contents	= s_oclbase;
	files[1].Filename	= "opencl-c-base.h";
	files[1].Length		= sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(files, 2, ARG, 1, &count, &spirv, 0,
			ACLSPV_CTX_FLAG0_SYMBOL);

	printf("%s: aclspv_compile: %d\n", c_tag, (int)RET);

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fprintf(stderr, "[%s] compile failed: %d\n", c_tag, (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fprintf(stderr, "[%s] malformed spir-v instruction stream\n", c_tag);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("%s: extinst=%u floor=%u targets_ok=%d\n"
			, c_tag, stats.m_num_extinst, stats.m_num_floor, stats.m_targets_ok);

	if(stats.m_num_floor < 1 || !stats.m_targets_ok) {
		fprintf(stderr, "[%s] Floor was not emitted\n", c_tag);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[%s] cannot write %s\n", c_tag, ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}

int main(void) {
	if(s_run(s_src_builtin_floorf, "floorf")) return EXIT_FAILURE;
	if(s_run(s_src_builtin_elementwise_floor, "elementwise_floor")) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
