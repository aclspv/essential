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

#ifndef util_op_h
#define util_op_h

#include <util/cxtp/vec.h>
#include "./literal.h"

enum ACLEXPR_OPBINCFG_ {
	ACLEXPR_OPBINCFG_NUMOPRND,

	/** @see ./literal.h */
	ACLEXPR_OPBINCFG_MASKLITERAL,
	ACLEXPR_OPBINCFG_SPECCONSTANTOP,
	ACLEXPR_OPBINCFG_RESTYID,
	ACLEXPR_OPBINCFG_RESID,
	ACLEXPR_OPBINCFG_OPCODE,
	ACLEXPR_OPBINCFG_END
};

enum ACLEXPR_OP_ {
	UTIL_OPBIN1_NONE,
	ACLEXPR_OP1_EXPLICIT_CAST,
	UTIL_OPBIN1_END
};

enum UTIL_OPBIN2_ {
	UTIL_OPBIN2_NONE,
	UTIL_OPBIN2_ADD,
	UTIL_OPBIN2_SUB,
	UTIL_OPBIN2_MUL,
	UTIL_OPBIN2_REM,
	UTIL_OPBIN2_DIV,
	UTIL_OPBIN2_ASSIGN
};

typedef struct {
	aclspv_wrd_t	m_type_original;
	aclspv_wrd_t	m_mask_literal;
	acltrl_t	m_literal;
	CXCursor	m_cursor;

	aclspv_bool_t	m_is_assign;
} aclexpr_opbincfg_extra;

/**
 * @brief type alias for `enum UTIL_OPBIN2_`
 * */
typedef uint_leastenum_t aclexpr_opbin2_t;

#endif
