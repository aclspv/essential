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
 * a Workgroup (`storage_class(4)`) pointer argument, addressed through.
 *
 * two things are pinned here.
 *
 * 1. the wrapper descent. the argument is materialised as an OpVariable whose
 *    pointee is a single-member Block wrapper — `ptr Workgroup struct { u32 }`
 *    — while the register the body sees has the plain `ptr Workgroup u32`
 *    logical type. the pointer VALUE is therefore member 0's address, i.e. an
 *    OpAccessChain of the variable, never the variable itself: copying the
 *    variable under the logical type is a type error the validator rejects:
 *
 *      %v = OpVariable %_ptr_Workgroup__struct_uint Workgroup
 *      %p = OpCopyObject %_ptr_Workgroup_uint %v      ; operand type != result
 *
 *    that is exactly what a builtin member 0 used to produce (it has no aclty
 *    node, so the descent bailed out and the emitter fell back to the copy).
 *
 * 2. the implicit int->uint conversion. the pointee is `u32` while the
 *    literals `7` and `1` are `int`, so each must materialise as
 *    `OpConstant %uint`. when the literal cast reused the signed constant the
 *    node kept its `int` type, the store layer's exact-type check rejected the
 *    store, and the store — not just its type — vanished from the module.
 *
 * the module must therefore hold one OpAccessChain into Workgroup space, two
 * OpStores whose pointer is that chain, and its `7` / `1` constants must be
 * unsigned; the spirv-val fixture attached to this target validates the
 * written file as well.
 * */
static const char s_src_wgptr[] =
"typedef unsigned int u32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"void t_wg(u32* w __attribute__((annotate(\"aclspv_storage_class(4)\")))) {\n"
"    *w = 7;\n"
"    *w = *w + 1;\n"
"}\n";

/** the opcode that defines `c_id`, or (unsigned)-1 when nothing does.
 *
 * a value is defined by the instruction whose third word (after the opcode
 * and the result type) is its id. */
static unsigned s_def_op(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_id
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			return (unsigned)-1;

		/** [op|wc][result type][result] .. — needs both to have a result. */
		if(wc >= 3 && rd[i + 2] == c_id)
			return (unsigned)(rd[i] & 0xffff);

		i += wc;
	}

	return (unsigned)-1;
}

/** the (first) result id of the first instruction with opcode `c_op`, or 0. */
static aclspv_wrd_t s_first_result(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		unsigned		c_op
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			return 0;

		if(wc >= 3 && (unsigned)(rd[i] & 0xffff) == c_op)
			return rd[i + 2];

		i += wc;
	}

	return 0;
}

/** the result id of `OpTypeInt 32 <c_signed>`, or 0. */
static aclspv_wrd_t s_int32_type(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_signed
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			return 0;

		/** OpTypeInt [op|4][result][width][signedness] */
		if((unsigned)(rd[i] & 0xffff) == (unsigned)SpvOpTypeInt
				&& wc == 4
				&& rd[i + 2] == 32u
				&& rd[i + 3] == c_signed)
			return rd[i + 1];

		i += wc;
	}

	return 0;
}

/** the result id of `OpConstant <c_ty> <c_val>`, or 0. */
static aclspv_wrd_t s_constant(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_ty,
		aclspv_wrd_t		c_val
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			return 0;

		/** OpConstant [op|4][result type][result][value] */
		if((unsigned)(rd[i] & 0xffff) == (unsigned)SpvOpConstant
				&& wc == 4
				&& rd[i + 1] == c_ty
				&& rd[i + 3] == c_val)
			return rd[i + 2];

		i += wc;
	}

	return 0;
}

/** the pointer operand of the first OpStore: [op|3][pointer][object]. */
static aclspv_wrd_t s_first_store_ptr(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			return 0;

		if((unsigned)(rd[i] & 0xffff) == (unsigned)SpvOpStore && wc == 3)
			return rd[i + 1];

		i += wc;
	}

	return 0;
}

/** the stored object of the first OpStore: [op|3][pointer][object]. */
static aclspv_wrd_t s_first_store_obj(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);

		if(!wc || i + wc > c)
			return 0;

		if((unsigned)(rd[i] & 0xffff) == (unsigned)SpvOpStore && wc == 3)
			return rd[i + 2];

		i += wc;
	}

	return 0;
}

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_chain;
	unsigned		c_store;
	aclspv_wrd_t		ty_uint;
	aclspv_wrd_t		ty_int;
	aclspv_wrd_t		id_var;
	aclspv_wrd_t		id_chain;
	aclspv_wrd_t		id_ptr;
	aclspv_wrd_t		id_u7;
	aclspv_wrd_t		id_obj;

	acltestcfg();

	files[0].Contents	= s_src_wgptr;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_wgptr) - 1;

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
		fprintf(stderr, "[wg-ptr] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[wg-ptr] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[wg-ptr] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_chain	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpAccessChain);
	c_store	= acltest_spirv_count_op(spirv, count, (unsigned)SpvOpStore);

	ty_uint	= s_int32_type(spirv, count, 0);
	ty_int	= s_int32_type(spirv, count, 1);

	id_var	= s_first_result(spirv, count, (unsigned)SpvOpVariable);
	id_chain = s_first_result(spirv, count, (unsigned)SpvOpAccessChain);
	id_ptr	= s_first_store_ptr(spirv, count);
	id_obj	= s_first_store_obj(spirv, count);
	id_u7	= s_constant(spirv, count, ty_uint, 7u);

	printf("accesschain=%u store=%u var=%u chain=%u storeptr=%u "
			"uint_ty=%u int_ty=%u uint_7=%u\n"
			, c_chain, c_store
			, (unsigned)id_var, (unsigned)id_chain, (unsigned)id_ptr
			, (unsigned)ty_uint, (unsigned)ty_int, (unsigned)id_u7);

	if(!ty_uint) {
		fputs("[wg-ptr] the module declares no 32-bit unsigned integer type\n"
				, stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** `7` is `int` in C but `u32` where it is stored, so the conversion must
	 * materialise `OpConstant %uint 7`. when the literal cast reused the
	 * signed constant there was no `%uint 7` at all — and the store that
	 * needed it was dropped. */
	if(!id_u7) {
		fputs("[wg-ptr] no OpConstant %uint 7: the int->uint conversion was "
				"not materialised\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!id_var) {
		fputs("[wg-ptr] no OpVariable in the module\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the argument must be reached through a chain: without one the body has
	 * no typed pointer into the wrapper, and the module does not validate. */
	if(c_chain < 1 || !id_chain) {
		fputs("[wg-ptr] no OpAccessChain; the argument register was copied "
				"straight out of the variable\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** `*w = 7` and `*w = *w + 1` must both reach memory. */
	if(c_store != 2 || !id_ptr) {
		fprintf(stderr, "[wg-ptr] stores: %u, expected 2 (`*w = 7` and "
				"`*w = *w + 1`)\n", c_store);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** a store whose pointer is the OpVariable result id means the wrapper was
	 * ignored and the module is mistyped. */
	if(id_ptr == id_var) {
		fputs("[wg-ptr] OpStore writes through the OpVariable, not the "
				"wrapper's member 0\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** and the stored object must be that unsigned constant, not a signed
	 * one: the store only validates if its object type matches the pointee. */
	if(id_obj != id_u7) {
		fprintf(stderr
				, "[wg-ptr] the first store writes id %u, expected the "
				  "unsigned constant %u\n"
				, (unsigned)id_obj, (unsigned)id_u7);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(s_def_op(spirv, count, id_ptr) != (unsigned)SpvOpAccessChain) {
		fprintf(stderr
				, "[wg-ptr] the stored pointer is not an OpAccessChain "
				  "(defined by opcode %u)\n"
				, s_def_op(spirv, count, id_ptr));
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[wg-ptr] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
