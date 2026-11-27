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
 * `OpName` for emitted symbols follows ACLSPV_CTX_FLAG0_SYMBOL.
 *
 * with the flag: add3 (a definition) and ext (an import declaration) are
 * named. without it: neither is, while the entry point keeps its name (it is
 * not a symbol, and its name is part of OpEntryPoint).
 * */
static const char s_src_norm_symbol[] =
"typedef int i32;\n"
"i32 add3(i32 a) { return a + 3; }\n"
"__attribute__((pure)) i32 ext(i32 x);\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_sym(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = add3(1) + ext(2);\n"
"}\n";

/** the string operand of the OpName at word i equals c_name. */
static int s_opname_is(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrdcount_t	i,
		const char*		c_name
		) {
	const unsigned		wc = (unsigned)(rd[i] >> 16);
	unsigned		k;

	for(k = 0; k < 256; ++k) {
		const unsigned		c_idx = 2u + (k >> 2);
		unsigned		c_byte;

		if(c_idx >= wc)
			return 0;

		c_byte = (unsigned)((rd[i + c_idx] >> ((k & 3u) << 3)) & 0xFFu);

		if(c_byte != (unsigned char)c_name[k])
			return 0;

		if(!c_name[k])
			return 1;
	}

	(void)c;
	return 0;
}

static int s_has_opname(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		const char*		c_name
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)SpvOpName && wc >= 3
				&& s_opname_is(rd, c, i, c_name))
			return 1;

		i += wc;
	}

	return 0;
}

/** compile once with c_flag0 and report which names are present. */
static int s_run(
		aclspv_wrd_t		c_flag0,
		int* ae2f_restrict	wr_add3,
		int* ae2f_restrict	wr_ext,
		int* ae2f_restrict	wr_entp
		) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	aclspv_compile_t	RET;

	acltestcfg();

	files[0].Contents	= s_src_norm_symbol;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_norm_symbol) - 1;

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
			, c_flag0
			);

	printf("aclspv_compile(flag0=0x%x): %d\n"
			, (unsigned)c_flag0, (int)RET);

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fputs("[norm-symbol] compile failed\n", stderr);
		free(spirv);
		return 1;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[norm-symbol] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return 1;
	}

	*wr_add3	= s_has_opname(spirv, count, "add3");
	*wr_ext		= s_has_opname(spirv, count, "ext");
	*wr_entp	= s_has_opname(spirv, count, "t_sym");

	printf("add3=%d ext=%d entry=%d\n", *wr_add3, *wr_ext, *wr_entp);

	/** the validator fixture reads the named module. */
	if(c_flag0) {
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[norm-symbol] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return 1;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return 0;
}

int main(void) {
	int c_add3, c_ext, c_entp;

	/** without the flag: only the entry point is named. */
	if(s_run(0, &c_add3, &c_ext, &c_entp))
		return EXIT_FAILURE;

	if(c_add3 || c_ext || !c_entp) {
		fputs("[norm-symbol] names without the flag\n", stderr);
		return EXIT_FAILURE;
	}

	/** with it: every emitted symbol is named. */
	if(s_run(ACLSPV_CTX_FLAG0_SYMBOL, &c_add3, &c_ext, &c_entp))
		return EXIT_FAILURE;

	if(!c_add3 || !c_ext || !c_entp) {
		fputs("[norm-symbol] missing names with the flag\n", stderr);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
