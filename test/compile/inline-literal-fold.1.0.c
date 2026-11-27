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
 * a literal argument to an `always_inline` callee keeps folding — on the way
 * in AND on the way out.
 *
 * a body whose only return is the last statement of its block is carved
 * straight into the caller's block: the return stores the result and leaves
 * the block open, so the parameter read is still the caller's SSA value and
 * `add3(4)` resolves to `4 + 3` at carve time. the literal then survives the
 * call in `aclemit_inline_fn`'s literal out-parameters, so `add3(4) + 1` folds
 * to the constant 8 and the entry point stores that constant.
 *
 * the scaffold path (several returns, or a return under a selection) still
 * runs its body behind a loop header, where every parameter read resolves
 * through a carry phi; that shape is covered by inline-multiret.
 * */
static const char* const s_src_lines[] = {
"typedef int i32;\n",
"__attribute__((always_inline)) i32 add3(i32 a) { return a + 3; }\n",
"__attribute__((always_inline)) i32 twice(i32 a) { return add3(a + 1); }\n",
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n",
"void entry(__attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out)\n",
"{ *out = add3(4) + 1; *out = twice(4); }\n"
};

static char s_src[1024];

/** opcodes this test reads that acltest/spirv.h does not name. */
#define	S_OP_ENTRY_POINT	15
#define	S_OP_CONSTANT		43
#define	S_OP_FUNCTION_END	56

/** the entry point's function id: OpEntryPoint is [op][model][id]. */
static aclspv_wrd_t s_entry_fn(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)S_OP_ENTRY_POINT && wc >= 3)
			return rd[i + 2];

		i += wc;
	}

	return 0;
}

/** is the constant `c_id`'s value `c_val`? OpConstant is
 * [op][result type][result id][value ...]. */
static int s_const_is(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_id,
		aclspv_wrd_t		c_val
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 0;

		if(op == (unsigned)S_OP_CONSTANT && wc >= 4 && rd[i + 2] == c_id)
			return rd[i + 3] == c_val;

		i += wc;
	}

	return 0;
}

struct s_entry_body {
	unsigned	m_num_iadd;
	unsigned	m_num_phi;
	unsigned	m_num_store;
	aclspv_wrd_t	m_stored[4];
};

/** what the entry point's own function body holds. the callee bodies are
 * still emitted as OpFunction (only their call sites are carved away), so
 * counting opcodes module-wide would see their arithmetic too. */
static int s_scan_entry(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_fn,
		struct s_entry_body*	wr
		) {
	aclspv_wrdcount_t i = 5;
	int in = 0;

	wr->m_num_iadd	= 0;
	wr->m_num_phi	= 0;
	wr->m_num_store	= 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 1;

		if(!in) {
			if(op == (unsigned)ACLTEST_SPV_OP_FUNCTION
					&& wc >= 5 && rd[i + 2] == c_fn)
				in = 1;
		}
		else if(op == (unsigned)S_OP_FUNCTION_END) {
			return 0;
		}
		else if(op == (unsigned)ACLTEST_SPV_OP_I_ADD)
			++wr->m_num_iadd;
		else if(op == (unsigned)ACLTEST_SPV_OP_PHI)
			++wr->m_num_phi;
		else if(op == (unsigned)ACLTEST_SPV_OP_STORE && wc >= 3) {
			if(wr->m_num_store < sizeof(wr->m_stored) / sizeof(wr->m_stored[0]))
				wr->m_stored[wr->m_num_store] = rd[i + 2];
			++wr->m_num_store;
		}

		i += wc;
	}

	/** no OpFunctionEnd: the entry was never reached. */
	return 1;
}

int main(void) {
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.c"};
	aclspv_compile_t RET;
	aclspv_wrd_t* spirv = 0;
	aclspv_wrdcount_t count = 0;
	unsigned calls;
	aclspv_wrd_t fn;
	struct s_entry_body B;
	acltest_spirv_stats stats;
	unsigned i;

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
		fprintf(stderr, "[inline-literal-fold] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats)) {
		fputs("[inline-literal-fold] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	calls = acltest_spirv_count_op(spirv, count,
			(unsigned)ACLTEST_SPV_OP_FUNCTION_CALL);

	if(calls != 0) {
		fprintf(stderr, "[inline-literal-fold] unexpected OpFunctionCall: %u\n", calls);
		free(spirv);
		return EXIT_FAILURE;
	}

	fn = s_entry_fn(spirv, count);

	if(!fn || s_scan_entry(spirv, count, fn, &B)) {
		fputs("[inline-literal-fold] entry point body not found\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	printf("entry: iadd=%u phi=%u store=%u\n",
			B.m_num_iadd, B.m_num_phi, B.m_num_store);

	/** neither the callee's `a + 3` nor the call site's `+ 1` may reach the
	 * module as an instruction, and no carry phi may appear either. */
	if(B.m_num_iadd || B.m_num_phi) {
		fputs("[inline-literal-fold] the inlined arithmetic was not folded\n",
				stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** `add3(4) + 1` and `twice(4)` both end at the constant 8. */
	if(B.m_num_store != 2) {
		fprintf(stderr, "[inline-literal-fold] unexpected OpStore count: %u\n",
				B.m_num_store);
		free(spirv);
		return EXIT_FAILURE;
	}

	for(i = 0; i < B.m_num_store && i < sizeof(B.m_stored) / sizeof(B.m_stored[0]); ++i) {
		if(!s_const_is(spirv, count, B.m_stored[i], 8)) {
			fprintf(stderr, "[inline-literal-fold] store %u is not the constant 8\n", i);
			free(spirv);
			return EXIT_FAILURE;
		}
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[inline-literal-fold] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
