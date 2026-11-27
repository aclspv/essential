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
#include <string.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * a write invalidates the cached value of the storage it may have landed in.
 *
 * there is no alias analysis here: `*p = 3` writes whatever `p` points at, and
 * `t = 7` is observable through a pointer taken from `t` earlier. each write
 * names a VAR, and a cached value carries the var's write generation it was
 * taken at, so a register that names the same var — through the view chain, a
 * pointer value, or another view of the same object — finds its cache stale on
 * the next read (aclcur_reg_val_current) and reloads.
 *
 * without it each side handed out a value from before the write:
 *
 *   i32 first = *p;   // 0
 *   *p = 3;           // writes t
 *   i32 second = *p;  // read 0 again, from the tracked ghost
 *   t = 7;            // writes t
 *   i32 third = *p;   // read 3 again, from the tracked ghost
 *
 * every one of those reads must come from memory, so the module holds three
 * OpLoads; the stale version folded them into one.
 * */
static const char* const s_src_lines[] = {
"typedef int i32;\n",
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n",
"void entry(__attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out)\n",
"{\n",
"    i32 t = 0;\n",
"    i32* p = &t;\n",
"    i32 first = *p;\n",
"    *p = 3;\n",
"    i32 second = *p;\n",
"    t = 7;\n",
"    i32 third = *p;\n",
"    *out = first + second + third;\n",
"}\n"
};

static char s_src[1024];

/** OpLoad. acltest/spirv.h does not name it. */
#define	S_OP_LOAD	61

int main(void) {
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.c"};
	aclspv_compile_t RET;
	aclspv_wrd_t* spirv = 0;
	aclspv_wrdcount_t count = 0;
	unsigned loads, iadds;
	acltest_spirv_stats stats;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	files[0].Contents = s_src;
	files[0].Filename = "main.c";
	files[0].Length = (unsigned)strlen(s_src);

	files[1].Contents = s_oclbase;
	files[1].Filename = "opencl-c-base.h";
	files[1].Length = sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(files, 2, ARG, 1, &count, &spirv, 0, 0);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fprintf(stderr, "[alias-invalidate] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats)) {
		fputs("[alias-invalidate] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	loads = acltest_spirv_count_op(spirv, count, (unsigned)S_OP_LOAD);
	iadds = acltest_spirv_count_op(spirv, count, (unsigned)ACLTEST_SPV_OP_I_ADD);

	printf("loads=%u iadds=%u\n", loads, iadds);

	/** three reads of `t` through the pointer, none of them cached across a
	 * write. */
	if(loads != 3) {
		fprintf(stderr, "[alias-invalidate] expected 3 OpLoad, got %u\n", loads);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the sum of the three loads: folding it would mean one of them was
	 * replaced by a constant. */
	if(iadds != 2) {
		fprintf(stderr, "[alias-invalidate] expected 2 OpIAdd, got %u\n", iadds);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!stats.m_targets_ok) {
		fputs("[alias-invalidate] a branch target names no label\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[alias-invalidate] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
