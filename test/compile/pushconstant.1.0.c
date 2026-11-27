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

#include <string.h>

/**
 * push-constant entry-point arguments.
 *
 * a push-constant argument is a pointer to a struct with
 * `aclspv_storage_class(9)`. each argument owns its OpVariable, so the block
 * layout — including its padding — belongs to that argument alone, and one
 * entry point's choice cannot constrain another's. the test checks
 *  - one PushConstant OpVariable per argument (three here: two in one entry
 *    point, one in another)
 *  - the Block wrapper structs exist
 *  - the bodies load the members through their own blocks
 *
 * note: the test targets are compiled with -DNDEBUG, so assert() is a
 * no-op here. every check must be an explicit branch returning failure.
 * */
static const char* const s_src_lines[] = {
	"typedef int i32;\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_pc(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(9)\")))\n",
	"    struct { i32 x; i32 pad; i32 y; }* a,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(9)\")))\n",
	"    struct { i32 z; }* b,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_v\n",
	") {\n",
	"    *out_v = a->x + a->y + b->z;\n",
	"}\n",
	"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
	"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n",
	"void t_pc2(\n",
	"    __attribute__((annotate(\"aclspv_storage_class(9)\")))\n",
	"    struct { i32 pad0; i32 only; }* c,\n",
	"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out_v\n",
	") {\n",
	"    *out_v = c->only;\n",
	"}\n",
};

/** C90 caps a concatenated string literal at 509 characters. */
static char s_src[1024];

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		num_pc;

	acltestcfg();

	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	files[0].Contents	= s_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(s_src);

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
		fprintf(stderr, "[pushconstant.1.0] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[pushconstant.1.0] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[pushconstant.1.0] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	num_pc = acltest_spirv_count_var_in(
			spirv, count, (aclspv_wrd_t)SpvStorageClassPushConstant);

	printf("inst=%u pc_var=%u type_struct=%u load=%u\n"
			, stats.m_num_inst
			, num_pc
			, acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT)
			, acltest_spirv_count_op(spirv, count, (unsigned)SpvOpLoad)
			);

	/** one block per push-constant argument, across entry points. */
	if(num_pc != 3) {
		fprintf(stderr, "[pushconstant.1.0] pc variables: %u, expected 3\n"
				, num_pc);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the Block wrappers, one per variable. */
	if(acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_TYPE_STRUCT) < 3) {
		fputs("[pushconstant.1.0] push-constant block structs missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the members are read through their own blocks: a->x, a->y, b->z,
	 * c->only. */
	if(acltest_spirv_count_op(spirv, count, (unsigned)SpvOpLoad) < 4
			|| acltest_spirv_count_op(spirv, count, ACLTEST_SPV_OP_LABEL) < 1) {
		fputs("[pushconstant.1.0] function body missing\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[pushconstant.1.0] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
