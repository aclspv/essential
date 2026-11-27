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

#include "./ctor.h"

#include <aclctx/err.h>

#include <aclexpr.h>
#include <aclval.h>
#include <aclval/nil.h>
#include <aclexpr/carve/ptr.h>
#include <aclexpr/carve/opn/call.h>
#include <aclexpr/util/unwrap.h>

#include <aclsym/fn.h>

#include <aclcur/common.h>
#include <aclcur/reg.h>
#include <aclcur/reg/var.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <aclutil/fn.h>


static enum CXChildVisitResult s_count_child(
		CXCursor	c,
		ae2f_unused CXCursor p,
		CXClientData	rw)
{
	(void)c;
	(void)p;
	++*((int*)rw);
	return CXChildVisit_Continue;
}


/** clear the destructor reservation on an l-value cursor. */
static void s_clear_dtor_lvalue(CXCursor c_arg, h_aclspv_ctx_t h_ctx) {
	CXCursor	SRC = aclexpr_expr_unwrap(c_arg);
	aclspv_id_t	REG;
	h_aclcur_t	p_reg;

	switch((aclspv_cxcurkind_t)clang_getCursorKind(SRC)) {
		default:
			return;

		case CXCursor_DeclRefExpr:
		case CXCursor_MemberRefExpr:
		case CXCursor_ArraySubscriptExpr:
			break;
	}

	REG = aclexpr_curreg_find(SRC, h_ctx);

	if(!REG)
		REG = aclexpr_curreg_find(clang_getCursorReferenced(SRC), h_ctx);

	p_reg = aclcur_get_node(REG, h_ctx);

	if(p_reg)
		p_reg->m_flag0 &= (aclspv_wrd_t)~aclcur_flag0_reg(
				ACLCUR_FLAG0_REG_DTOR);
}

/** address of a constructor source argument. references are lowered to
 * pointers, so the constructor receives the object's storage address. */
static aclspv_bool_t s_arg_addr(
		CXCursor	c_arg,
		aclspv_id_t*	wr_id,
		aclty_id_t*	wr_ty,
		h_aclspv_ctx_t	h_ctx
		) {
	CXCursor	SRC;
	aclspv_id_t	REG;
	h_aclcur_t	p_reg;
	const aclty_t* ae2f_restrict p_ty;

	c_arg = aclexpr_expr_unwrap(c_arg);

	if(aclutil_cursor_is_std_move(c_arg)) {
		CXCursor MOVE_ARG = clang_Cursor_getArgument(c_arg, 0);

		s_clear_dtor_lvalue(MOVE_ARG, h_ctx);
		c_arg = MOVE_ARG;
	}

	SRC = aclexpr_expr_unwrap(c_arg);

	if(clang_getCursorKind(SRC) == CXCursor_CXXThisExpr) {
		if(!h_ctx->m_this)
			return 0;

		ae2f_ifezerr(p_reg = aclcur_get_node(h_ctx->m_this, h_ctx))
			return 0;

		ae2f_ifezerr(*wr_id = aclexpr_ptrval(h_ctx->m_this, h_ctx))
			return 0;

		*wr_ty = p_reg->m_ty;
		return 1;
	}

	unless(clang_getCursorKind(SRC) == CXCursor_DeclRefExpr)
		return 0;

	ae2f_ifezerr(REG = aclexpr_curreg_find(
				clang_getCursorReferenced(SRC), h_ctx))
		return 0;

	ae2f_ifezerr(p_reg = aclcur_get_node(REG, h_ctx))
		return 0;

	ae2f_ifezerr(p_ty = aclty_find(p_reg->m_ty, h_ctx))
		return 0;

	if(p_ty->m_class == ACLTY_PTR) {
		ae2f_ifezerr(*wr_id = aclexpr_ptrval(REG, h_ctx))
			return 0;

		*wr_ty = p_reg->m_ty;
		return 1;
	}

	ae2f_ifezerr(*wr_ty = aclty_mk_ptr(
				p_reg->m_ty, SpvStorageClassFunction, h_ctx))
		return 0;

	aclexpr_ptr_taken(REG, h_ctx);

	ae2f_ifezerr(*wr_id = aclcur_reg_addr(REG, h_ctx))
		return 0;

	return 1;
}

/** call a user-provided copy/move constructor into c_dest. */
static aclspv_bool_t s_call_copy_ctor(
		CXCursor	c_call,
		aclspv_id_t	c_dest,
		h_aclspv_ctx_t	h_ctx
		) {
	CXCursor			c_ctor = clang_getCursorReferenced(c_call);
	aclsym_fn_t* ae2f_restrict	p_sym;
	aclty_t* ae2f_restrict	p_fn;
	aclspv_id_t		id_fn_ty, id_dest, id_call;
	aclspv_id_t		args[__ACLSPV_MAX_OPRND];
	aclspv_wrd_t		c_n = 1, c_i;
	aclspv_bool_t		is_pure = 0;
	h_aclcur_t		p_dest;
	int			NARG;

	unless(aclutil_cursor_is_copy_move_ctor_call(c_call))
		return 0;

	unless(aclutil_ctor_has_body(c_ctor))
		return 0;

	p_sym = aclsym_fn_find(c_ctor, h_ctx);

	unless(p_sym)
		return 0;

	if(!(p_sym->m_flag0 & ACLSYM_FN_FLAG0_CALLABLE)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(
				c_ctor, clang_getCursorType(c_ctor), h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(p_fn = aclty_find(id_fn_ty, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(p_dest = aclcur_get_node(c_dest, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	aclexpr_ptr_taken(c_dest, h_ctx);

	ae2f_ifezerr(id_dest = aclcur_reg_addr(c_dest, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	args[0] = id_dest;

	NARG = clang_Cursor_getNumArguments(c_call);

	if(NARG < 0) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	for(c_i = 0; c_i < (aclspv_wrd_t)NARG; ++c_i) {
		const CXCursor		c_arg = clang_Cursor_getArgument(c_call, (unsigned)c_i);
		const CXType		TY_ARG = clang_getArgType(
				clang_getCursorType(c_ctor), (unsigned)c_i);
		aclty_t* ae2f_restrict	p_arg;
		aclspv_id_t		id_arg;
		aclty_id_t		ty_arg;

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_fn, c_i + 1, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 1;
		}

		if(TY_ARG.kind == CXType_LValueReference
				|| TY_ARG.kind == CXType_RValueReference) {
			if(!s_arg_addr(c_arg, &id_arg, &ty_arg, h_ctx)) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return 1;
			}
		}
		else {
			acltrl_t lit;

			lit.m_api_uintmax = 0;

			switch(aclexpr_eval(
						&id_arg
						, p_arg->m_info.m_fn_arg.m_prm_id
						, c_arg
						, &lit
						, h_ctx)) {
				default:
				case ACLEXPR_RET_NOT_THE_CASE:
				case ACLEXPR_RET_FAILURE:
					_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
					return 1;

				case ACLEXPR_RET_SUCCESS:
				case ACLEXPR_RET_SUCCESS_CONSTANT:
				case ACLEXPR_RET_SUCCESS_LITERAL:
					break;
			}
		}

		args[c_n++] = id_arg;
	}

	ae2f_ifnezerr(aclexpr_call_make(
				p_sym->m_id
				, p_fn->m_info.m_fn.m_ret_id
				, args
				, c_n
				, &id_call
				, &is_pure
				, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	(void)id_call;


	return 1;
}

aclspv_bool_t aclexpr_copy_ctor_value(
		CXCursor	c_call,
		aclspv_id_t*	wr_id,
		aclty_id_t*	wr_ty,
		h_aclspv_ctx_t	h_ctx
		) {
	aclty_id_t	ty;
	aclspv_id_t	dest;

	unless(aclutil_cursor_is_copy_move_ctor_call(c_call))
		return 0;

	ae2f_ifezerr(ty = aclty_mk_by_cursor_default(
				c_call, clang_getCursorType(c_call), h_ctx))
		return 0;

	{
		const aclcur_reg_t REGIN = aclcur_regvar_in();

		ae2f_ifezerr(dest = aclcur_cfg_reg(
					clang_getNullCursor()
					, ty
					, SpvStorageClassFunction
					, &REGIN
					, h_ctx))
			return 0;
	}

	unless(s_call_copy_ctor(c_call, dest, h_ctx))
		return 0;

	ae2f_ifezerr(*wr_id = aclcur_load_reg(dest, h_ctx, 0))
		return 0;

	*wr_ty = ty;
	return 1;
}

aclspv_bool_t aclexpr_varinit(
		CXCursor	c_init,
		aclspv_id_t	c_obj,
		aclty_id_t	c_ty,
		aclspv_bool_t*	wr_constructed,
		h_aclspv_ctx_t	h_ctx
		) {
	CXCursor		c_call, c_ctor;
	aclsym_fn_t* ae2f_restrict p_sym;
	aclty_t* ae2f_restrict	p_fn;
	aclspv_id_t		id_fn_ty, id_obj_ty, id_call;
	aclspv_id_t		args[__ACLSPV_MAX_OPRND];
	aclspv_wrd_t		c_n, c_i;
	aclspv_bool_t		is_pure = 0;
	h_aclcur_t		p_obj;
	int			NARG;
	acltrl_t		lit;

	ae2f_ifezerr(h_ctx)	return 0;

	if(wr_constructed)
		*wr_constructed = 0;

	c_call = aclexpr_expr_unwrap(c_init);

	/** an empty brace initialisation of a trivial type: value initialise
	 * it to null. a non-empty list keeps the ordinary init-list lane. */
	if(clang_getCursorKind(c_call) == CXCursor_InitListExpr) {
		int			c_num_child = 0;
		aclspv_id_t		id_nil;

		clang_visitChildren(c_call, s_count_child, &c_num_child);

		if(c_num_child)
			return 0;

		ae2f_ifezerr(id_nil = aclval_cfg_nil(c_ty, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT);
			return 1;
		}

		ae2f_ifezerr(aclcur_store_reg(c_obj, id_nil, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 1;
		}

		return 1;
	}

	if(clang_getCursorKind(c_call) != CXCursor_CallExpr)
		return 0;

	c_ctor = clang_getCursorReferenced(c_call);

	if(clang_getCursorKind(c_ctor) != CXCursor_Constructor)
		return 0;

	switch(aclutil_varinit_kind(c_call)) {
		default:
		case ACLUTIL_VARINIT_NONE:
			/** `S s;`: intentionally uninitialised. */
			return 1;

		case ACLUTIL_VARINIT_CTOR:
			break;

		case ACLUTIL_VARINIT_VALUE:
			/** `S s = ...`: a user-provided copy/move constructor runs on
			 * the destination; otherwise this is a plain value copy. */
			if(s_call_copy_ctor(c_call, c_obj, h_ctx)) {
				if(wr_constructed)
					*wr_constructed = 1;

				return 1;
			}
			{
		aclspv_id_t	id_init;
		acltrl_t	tmp_lit;

		NARG = clang_Cursor_getNumArguments(c_call);

		if(NARG == 0) {
			ae2f_ifezerr(id_init = aclval_cfg_nil(c_ty, h_ctx)) {
				_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT);
				return 1;
			}
		}
		else if(NARG == 1) {
			const CXCursor c_arg = clang_Cursor_getArgument(c_call, 0);

			tmp_lit.m_api_uintmax = 0;

			switch(aclexpr_eval(
						&id_init, c_ty, c_arg, &tmp_lit, h_ctx)) {
				default:
				case ACLEXPR_RET_NOT_THE_CASE:
				case ACLEXPR_RET_FAILURE:
					_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
					return 1;

				case ACLEXPR_RET_SUCCESS:
				case ACLEXPR_RET_SUCCESS_CONSTANT:
				case ACLEXPR_RET_SUCCESS_LITERAL:
					break;
			}
		}
		else {
			err_puts("[aclexpr::varinit] value initialisation has too many arguments");
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 1;
		}

		ae2f_ifezerr(aclcur_store_reg(c_obj, id_init, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 1;
		}

		return 1;
			}
	}

	/** an implicit / defaulted constructor has no body: leave the object
	 * uninitialised, exactly as C would. */
	unless(aclutil_ctor_has_body(c_ctor))
		return 1;

	p_sym = aclsym_fn_find(c_ctor, h_ctx);

	unless(p_sym)
		return 1;

	if(!(p_sym->m_flag0 & ACLSYM_FN_FLAG0_CALLABLE)) {
		err_puts("[aclexpr::varinit] constructor is not emitted yet");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(
				c_ctor
				, clang_getCursorType(c_ctor)
				, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(p_fn = aclty_find(id_fn_ty, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(p_obj = aclcur_get_node(c_obj, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	ae2f_ifezerr(id_obj_ty = aclty_mk_ptr(
				p_obj->m_ty
				, SpvStorageClassFunction
				, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_ID);
		return 1;
	}

	aclexpr_ptr_taken(c_obj, h_ctx);

	ae2f_ifezerr(args[0] = aclcur_reg_addr(c_obj, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	(void)id_obj_ty;	/** the function type owns the matching this type */

	NARG = clang_Cursor_getNumArguments(c_call);

	if(NARG < 0) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	c_n = 1;

	for(c_i = 0; c_i < (aclspv_wrd_t)NARG; ++c_i) {
		const CXCursor		c_arg = clang_Cursor_getArgument(c_call, (unsigned)c_i);
		aclty_t* ae2f_restrict	p_arg;
		aclspv_id_t		id_arg;

		ae2f_ifnezerr_strict(clang_Cursor_isNull(c_arg)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 1;
		}

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_fn, c_i + 1, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 1;
		}

		lit.m_api_uintmax = 0;

		switch(aclexpr_eval(
					&id_arg
					, p_arg->m_info.m_fn_arg.m_prm_id
					, c_arg
					, &lit
					, h_ctx)) {
			default:
			case ACLEXPR_RET_NOT_THE_CASE:
			case ACLEXPR_RET_FAILURE:
				_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
				return 1;

			case ACLEXPR_RET_SUCCESS:
			case ACLEXPR_RET_SUCCESS_CONSTANT:
			case ACLEXPR_RET_SUCCESS_LITERAL:
				break;
		}

		args[c_n++] = id_arg;
	}

	ae2f_ifnezerr(aclexpr_call_make(
				p_sym->m_id
				, p_fn->m_info.m_fn.m_ret_id
				, args
				, c_n
				, &id_call
				, &is_pure
				, h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 1;
	}

	(void)id_call;

	if(wr_constructed)
		*wr_constructed = 1;

	return 1;
}