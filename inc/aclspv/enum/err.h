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

#ifndef	aclspv_enum_err_h
#define aclspv_enum_err_h

#include "../wrd.h"

enum ACLSPV_COMPILE_ {
	ACLSPV_COMPILE_OK,
	ACLSPV_COMPILE_PRM_INVAL_NIL,

	ACLSPV_COMPILE_NOIMPL,
	ACLSPV_COMPILE_NOIMPL_V_WRKGROUP_SIZE,
	ACLSPV_COMPILE_NOIMPL_CAPABILITY,
	ACLSPV_COMPILE_NOIMPL_EXTENSION,

	ACLSPV_COMPILE_ALLOC_FAILED,
	ACLSPV_COMPILE_ALLOC_FAILED_EMIT_WRD,
	ACLSPV_COMPILE_ALLOC_FAILED_CURSOR,
	ACLSPV_COMPILE_ALLOC_FAILED_CFG,
	ACLSPV_COMPILE_ALLOC_FAILED_EXPR,
	ACLSPV_COMPILE_ALLOC_FAILED_DECLGLOBOBJ,
	ACLSPV_COMPILE_ALLOC_FAILED_VEC,
	ACLSPV_COMPILE_ALLOC_FAILED_ID,
	ACLSPV_COMPILE_ALLOC_FAILED_ASM,
	ACLSPV_COMPILE_ALLOC_FAILED_CAPABILITY,
	ACLSPV_COMPILE_ALLOC_FAILED_CXTP,
	ACLSPV_COMPILE_ALLOC_FAILED_ENTP,
	ACLSPV_COMPILE_ALLOC_FAILED_ENTP_ARGS,
	ACLSPV_COMPILE_ALLOC_FAILED_FUNCTION,
	ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT,

	ACLSPV_COMPILE_MET_INVAL,
	ACLSPV_COMPILE_MET_INVAL_BRK,
	ACLSPV_COMPILE_MET_INVAL_CONTINUE,
	ACLSPV_COMPILE_MET_INVAL_UNREACHABLE,
	ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS,
	ACLSPV_COMPILE_MET_INVAL_BODY_NOKIND,
	ACLSPV_COMPILE_MET_INVAL_GOTO_NOT_LABEL,
	ACLSPV_COMPILE_MET_INVAL_TYPE_NOT_KNOWN,
	ACLSPV_COMPILE_MET_INVAL_BODY_VARDECL_GET_EXPR,

	ACLSPV_COMPILE_ERR_CLANG,
	ACLSPV_COMPILE_ERR_CXIDX,
	ACLSPV_COMPILE_ERR_CXTU,

	ACLSPV_COMPILE_TOO_BIG,
	ACLSPV_COMPILE_TOO_BIG_EXPRVISIT,
	ACLSPV_COMPILE_TOO_BIG_CUR_LBL,

	ACLSPV_COMPILE_CXTP_ZERO,
	ACLSPV_COMPILE_CXTP_STRUCT_MBR_NUM_IS_LYING,
	ACLSPV_COMPILE_CXTP_FN_ARGS_NUM_IS_LYING,

	ACLSPV_COMPILE_CXTP_TYINVL_STRUCT_DCR,
	ACLSPV_COMPILE_CXTP_TYINVL_ARR_DCR,

	ACLSPV_COMPILE_WORKGROUP_MUST_BE_CONSTANT_ARRAY,
	ACLSPV_COMPILE_WORKGROUP_SIZE_NO_INT,
	ACLSPV_COMPILE_STORAGE_CLASS_UNDEFINED,
	ACLSPV_COMPILE_BTCAST_NON_NUMBER,
	ACLSPV_COMPILE_CURSOR_NOT_FOUND,
	ACLSPV_COMPILE_IF_COND_INVAL,
	ACLSPV_COMPILE_INSUFFICIENT_BUFFER_LENGTH,

	ACLSPV_COMPILE_WHILE_LACK_COMPONENTS,
	ACLSPV_COMPILE_FOR_LACK_COMPONENTS,
	ACLSPV_COMPILE_INFINITE_LOOP_FOUND,

	/** switch body is not a form this emitter can carve (no case label,
	 * a selector that is not an integer scalar, ...). */
	ACLSPV_COMPILE_SWITCH_LACK_COMPONENTS,
	ACLSPV_COMPILE_SWITCH_SELECTOR_INVAL,
	ACLSPV_COMPILE_SWITCH_CASE_INVAL,
	ACLSPV_COMPILE_UNHANDLED_SWITCH,

	ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD,
	ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW,
	ACLSPV_COMPILE_ARITHMETIC_CAST_ID_IS_UNDEFINED_OLD,

	ACLSPV_COMPILE_ASSIGN_ON_NON_LVAL,
	ACLSPV_COMPILE_ASSIGN_CAST_GOT_ZERO,

	ACLSPV_COMPILE_UNHANDLED,
	ACLSPV_COMPILE_UNHANDLED_ARGS,
	ACLSPV_COMPILE_UNHANDLED_BODY,
	ACLSPV_COMPILE_UNHANDLED_BODY_LBL,
	ACLSPV_COMPILE_UNHANDLED_BODY_CURKINDER,
	ACLSPV_COMPILE_UNHANDLED_BODY_VARDECL,
	ACLSPV_COMPILE_UNHANDLED_BODY_RET,
	/** a statement expression whose block produced no value. */
	ACLSPV_COMPILE_UNHANDLED_BODY_STMTEXPR,
	ACLSPV_COMPILE_UNHANDLED_WHILE,
	ACLSPV_COMPILE_UNHANDLED_FOR,
	ACLSPV_COMPILE_UNHANDLED_VISIT,
	ACLSPV_COMPILE_UNHANDLED_CARVE,

	ACLSPV_COMPILE_DIAG_FTAL,
	ACLSPV_COMPILE_DIAG_ERR,

	ACLSPV_COMPILE_NOT_INIT_STATE,
	ACLSPV_COMPILE_PUSHCONSTANT_TOO_BIG,

	/** pointer arithmetic (`p + n`, `p - n`, `p - q`, `++p`, `--p`) is a
	 * deliberate language-model rejection in the logical addressing model,
	 * not a missing feature: pointers are plain ids there, with no
	 * address to offset. physical addressing will need it later, so this
	 * is gated on ACLSPV_CTX_FLAG0_LOGICAL, never unconditional. */
	ACLSPV_COMPILE_POINTER_ARITHMETIC,

	/** dereference / address-of applied to something that is not a
	 * pointer. an explicit rejection, never a silent pass: emitting a load
	 * or a store through a non-pointer id produces a module that is
	 * structurally wrong rather than diagnosable. */
	ACLSPV_COMPILE_NOT_A_POINTER,

	/** see ACLSPV_CTX_FLAG0_ERROR_UNASSIGNED_VAL_USED */
	ACLSPV_COMPILE_UNASSIGNED_VALUE,

	/** see ACLSPV_CTX_FLAG0_EXPLICIT_EXTINST_IMPORT */
	ACLSPV_COMPILE_NOIMPL_EXTINST,

	/** two declarations of the same name do not share a canonical function
	 * type. the language model has no overloading, so the conflict is
	 * rejected before any body is parsed. see aclemit_fn_gather(). */
	ACLSPV_COMPILE_MET_INVAL_FN_REDECL,

	/** build phase: a deferred body never becomes emittable — a call cycle
	 * (recursion), or a callee that never got a body. */
	ACLSPV_COMPILE_RECURSIVE_CALL,

	/** the module already declares this symbol as an import, and this pass
	 * found a body for it: `LinkageAttributes Import` promises the definition
	 * comes from another module, so emitting one here would contradict the
	 * declaration the module already carries.
	 *
	 * a function is imported by the pass that sees a call to it with no body,
	 * so this says the passes disagree about where the function lives. */
	ACLSPV_COMPILE_IMPORT_DEFINED,

	/** a subobject destructor cannot be called by pointer and is not
	 * inlinable. mark it inline/always_inline or avoid the subobject. */
	ACLSPV_COMPILE_SUBOBJECT_DTOR_NOT_INLINE
};

/** 
 * could now fit in 32-bit.
 * see enum ACLSPV_COMPILE_
 * */
typedef aclspv_wrd_t	aclspv_compile_t;
typedef int ACLSPV_COMPILE_SIZE_CHECK[sizeof(enum ACLSPV_COMPILE_) == sizeof(aclspv_wrd_t) ? 1 : -1];

#endif
