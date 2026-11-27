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

#include "./sync.h"

#include <aclctx/err.h>

#include <aclcur/common.h>
#include <aclcur/op.h>

#include <string.h>

/** one barrier's shape. */
struct s_sync {
	/** the builtin's spelling, as the caller writes it. */
	const char*	m_name;
	enum SpvOp_	m_op;
	/** the instruction's word count, which is also the builtin's argument
	 * count. */
	aclspv_wrd_t	m_narg;
};

/** the synchronisation builtins and the instruction each one emits. */
static const struct s_sync S_SYNC[] = {
	/** scope_exec, scope_mem, mem_sem */
	{ "aclspv_ctlbar",	SpvOpControlBarrier,	3 },
	/** scope_mem, mem_sem */
	{ "aclspv_membar",	SpvOpMemoryBarrier,	2 }
};

#define	S_NUM_SYNC	((aclspv_wrd_t)(sizeof(S_SYNC) / sizeof(S_SYNC[0])))

static const struct s_sync* s_find(const char* c_name) {
	aclspv_wrd_t i;

	ae2f_ifezerr(c_name) return 0;

	for(i = 0; i < S_NUM_SYNC; ++i)
		if(!strcmp(c_name, S_SYNC[i].m_name))
			return &S_SYNC[i];

	return 0;
}

/** the constant word a barrier argument folds to.
 *
 * a barrier takes scopes and semantics masks, never a value: an argument that
 * does not evaluate to a non-negative integer that fits a word is a
 * rejection, not a runtime operand. */
static aclspv_bool_t s_arg_word(
		const CXCursor			c_arg,
		aclspv_wrd_t* ae2f_restrict	r_word,
		h_aclspv_ctx_t			h_ctx
		) {
	const CXEvalResult	EVAL = clang_Cursor_Evaluate(c_arg);
	aclspv_wrd2_t		VAL;

	if(!EVAL) {
		err_puts("[aclemit::sync] a barrier argument is not a constant");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
		return 1;
	}

	if(clang_EvalResult_getKind(EVAL) != CXEval_Int) {
		err_puts("[aclemit::sync] a barrier argument is not an integer");
		clang_EvalResult_dispose(EVAL);
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
		return 1;
	}

	VAL = (aclspv_wrd2_t)clang_EvalResult_getAsUnsigned(EVAL);
	clang_EvalResult_dispose(EVAL);

	/** a scope / semantics word is one word wide. */
	if(VAL > 0xFFFFFFFFu) {
		err_puts("[aclemit::sync] a barrier argument does not fit in a word");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
		return 1;
	}

	*r_word = (aclspv_wrd_t)VAL;
	return 0;
}

aclspv_bool_t aclemit_sync_is_barrier(CXCursor c_cur) {
	CXString			SPL;
	aclspv_bool_t			RET;

	unless((aclspv_cxcurkind_t)clang_getCursorKind(c_cur) == CXCursor_CallExpr)
		return 0;

	SPL = clang_getCursorSpelling(c_cur);
	RET = (aclspv_bool_t)(s_find((const char*)SPL.data) != 0);
	clang_disposeString(SPL);

	return RET;
}

aclspv_bool_t aclemit_sync_barrier(CXCursor c_cur, h_aclspv_ctx_t h_ctx) {
	const struct s_sync*	p_sync;
	CXString		SPL;
	/* every element is written before use: the count is the instruction's
	 * word count, which is at most 3. */
	aclspv_wrd_t		WORDS[4] = { 0, 0, 0, 0 };
	aclspv_wrd_t		i;

	ae2f_ifezerr(h_ctx) return 1;

	unless((aclspv_cxcurkind_t)clang_getCursorKind(c_cur) == CXCursor_CallExpr) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
		return 1;
	}

	SPL = clang_getCursorSpelling(c_cur);
	p_sync = s_find((const char*)SPL.data);
	clang_disposeString(SPL);

	unless(p_sync) {
		err_puts("[aclemit::sync] not a barrier call");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
		return 1;
	}

	ae2f_ifnezerr_strict(clang_Cursor_getNumArguments(c_cur)
			!= (int)p_sync->m_narg) {
		err_prefix();
		err_call((void), fprintf(stderr
					, "[aclemit::sync] %s takes %u argument(s)\n"
					, p_sync->m_name
					, (unsigned)p_sync->m_narg));

		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
		return 1;
	}

	/** every argument is checked BEFORE the instruction's node exists: a
	 * node is appended to the arena, and a later instruction of the block
	 * (the next statement's) must not land in front of it. */
	for(i = 0; i < p_sync->m_narg; ++i) {
		const CXCursor ARG = clang_Cursor_getArgument(c_cur, (unsigned)i);

		ae2f_ifnezerr_strict(clang_Cursor_isNull(ARG)) {
			err_puts("[aclemit::sync] a barrier argument is missing");
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_BODY);
			return 1;
		}

		if(s_arg_word(ARG, &WORDS[i], h_ctx))
			return 1;
	}

	ae2f_ifezerr(aclcur_cfg_barrier(p_sync->m_op, WORDS, p_sync->m_narg, h_ctx))
		return 1;

	return 0;
}
