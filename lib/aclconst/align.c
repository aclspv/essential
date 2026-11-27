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

#include "./align.h"
#include <.aclspv/const/spec.h>
#include <ae2f/Keys.h>


#define ADD(r, c_x, c_y) {	\
	(r).m_id = 0u;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd + (c_y).m_lit.m_wrd;	\
}

#define SUB(r, c_x, c_y) {	\
	(r).m_id = 0u;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd - (c_y).m_lit.m_wrd;	\
}

#define RSHFT(r, c_x, c_y) {	\
	(r).m_id = 0u;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd >> (c_y).m_lit.m_wrd;	\
}

#define OR(r, c_x, c_y) {	\
	(r).m_id = 0u;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd | (c_y).m_lit.m_wrd;	\
}
#define AND(r, c_x, c_y) {	\
	(r).m_id = 0u;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd & (c_y).m_lit.m_wrd;	\
}



#define NOT(r, c) {				\
	(r).m_id = 0u;				\
	(r).m_lit.m_max = ~(c).m_lit.m_max;	\
}


aclconst_specsz	aclconst_alignup(
		aclconst_specsz	c,
		aclconst_specsz	c_algn,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz algn1, v1, n_algn1, vadd, ret;

	v1.m_id = 0;
	v1.m_lit.m_wrd = 1;

	_aclconst_specsize_op2(L, algn1, c_algn, v1, SpvOpISub, SUB, h_ctx);
	_aclconst_specsize_op2(L, vadd, algn1, c, SpvOpIAdd, ADD, h_ctx);
	_aclconst_specsize_op1(L, n_algn1, algn1, SpvOpSNegate, NOT, h_ctx);
	_aclconst_specsize_op2(L, ret, n_algn1, vadd, SpvOpBitwiseAnd, AND, h_ctx);

	return ret;
}

aclconst_specsz	aclconst_align_from_size(
		aclconst_specsz	c,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz op0, op1, res;
	aclspv_wrd_t top = h_ctx->m_usrcfg.m_byte_size_type << 3, b;

	op1.m_id	= 0;
	op1.m_lit.m_wrd	= 1;
	_aclconst_specsize_op2(L, res, c, op1, SpvOpISub, SUB, h_ctx);

	for(b = 1; b < top; b <<= 1) {
		op0 = res;
		op1.m_id	= 0;
		op1.m_lit.m_wrd	= b;
		_aclconst_specsize_op2(L, res, op0, op1, SpvOpShiftRightLogical, RSHFT, h_ctx);

		op1 = res;
		_aclconst_specsize_op2(L, res, op0, op1, SpvOpBitwiseOr, OR, h_ctx);
	}

	op0 = res;
	op1.m_id	= 0;
	op1.m_lit.m_wrd	= 1;
	_aclconst_specsize_op2(L, res, op0, op1, SpvOpIAdd, ADD, h_ctx);
	return res;
}

aclconst_specsz	aclconst_align_from_size2(
		aclspv_wrd_t	c,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz	co;
	co.m_id		= 0;
	co.m_lit.m_wrd	= c;

	return aclconst_align_from_size(co, h_ctx);
}
