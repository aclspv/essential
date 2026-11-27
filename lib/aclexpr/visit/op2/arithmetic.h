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

/** 
 * @file	basic_numeric_operation.h
 * @brief	`+, -, *, /, %`
 * */

#ifndef	aclexpr_visit_basic_numeric_operation_h
#define aclexpr_visit_basic_numeric_operation_h

#include <aclutil/scale.h>
#include <util/log.h>

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/op.h>

#include <aclutil/emitx.h>

#include <aclmem.h>

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op2_arithmetic(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		)
{
	enum ACLEXPR_OP_	OPCODE;

	ae2f_expected_but_else(h_ctx) {
		return c_lbl_fail;
	}

	switch((aclspv_cxcurkind_t)clang_getCursorKind(c_cur)) {
		default:
			return c_lbl_pass;

		case CXCursor_BinaryOperator:
		case CXCursor_CompoundAssignOperator:
			break;
	}

	switch((aclspv_cxbinopkind_t)clang_getCursorBinaryOperatorKind(c_cur)) {
		default:
			return c_lbl_pass;

		case CXBinaryOperator_AddAssign:
			OPCODE = ACLEXPR_OP2_ADDASS;
			break;
		case CXBinaryOperator_Add:
			OPCODE = ACLEXPR_OP2_ADD;
			break;

		case CXBinaryOperator_SubAssign:
			OPCODE = ACLEXPR_OP2_SUBASS;
			break;

		case CXBinaryOperator_Sub:
			OPCODE = ACLEXPR_OP2_SUB;
			break;

		case CXBinaryOperator_MulAssign:
			OPCODE = ACLEXPR_OP2_MULASS;
			break;
		case CXBinaryOperator_Mul:
			OPCODE = ACLEXPR_OP2_MUL;
			break;

		case CXBinaryOperator_DivAssign:
			OPCODE = ACLEXPR_OP2_DIVASS;
			break;
		case CXBinaryOperator_Div:
			OPCODE = ACLEXPR_OP2_DIV;
			break;

		case CXBinaryOperator_RemAssign:
			OPCODE = ACLEXPR_OP2_REMASS;
			break;
		case CXBinaryOperator_Rem:
			OPCODE = ACLEXPR_OP2_REM;
			break;
	}

	{
		aclspv_wrd_t* ae2f_restrict VEC;
		x_scale* ae2f_restrict	NEW_SCALE;

		NEW_SCALE = get_last_scale_from_vec(h_cmdscale);

		NEW_SCALE = mk_scale_from_vec(
				h_cmdscale
				, 
				aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 2)
				+ (sizeof(aclexpr_opbincfg_extra) * 2)
				);
		ae2f_expected_but_else(NEW_SCALE)
			return c_lbl_fail;

		VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

		ae2f_expected_but_else(VEC)
			return c_lbl_fail;


		VEC[ACLEXPR_OPBINCFG_NUMOPRND] = 2;				/** number of operands */
		VEC[ACLEXPR_OPBINCFG_SPECCONSTANTOP] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[ACLEXPR_OPBINCFG_OPCODE] = (aclspv_wrd_t) OPCODE;

		aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 2], sizeof(aclexpr_opbincfg_extra) << 1);
	}

	return c_lbl_success;
}

#endif
