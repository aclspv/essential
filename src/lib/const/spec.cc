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

#ifndef aclspv_const_spec_cc
#define aclspv_const_spec_cc

#include <aclval.h>

#include <aclconst/specsize.h>
#include <aclconst/specop.h>

#include <ae2f/pat/macrofunc.h>
#include <aclutil/emitx.h>

#include <aclctx/err.h>

#include <aclty/sclr.h>

ae2f_MAC((L, )) aclconst_specsize_op3_sel(
		aclconst_specsz&	r,
		const aclconst_specsz	c_cnd,
		const aclconst_specsz	c_t,
		const aclconst_specsz	c_f,
		h_aclspv_ctx_t		h_ctx
		) { do {
	aclspv_id_t		L$$size_ty = aclty_mk_default(
			aclty_get_sclr_uint(
				(h_ctx)->m_usrcfg.m_byte_size_type << 3u
				)
			, h_ctx
			);

	ae2f_ifnezerr_strict(0u == L$$size_ty) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
		(r).m_id	= 0u;
		(r).m_lit.m_wrd	= 0u;
		break;
	}

	unless((c_cnd).m_id) {
		(r) = (c_cnd).m_lit.m_wrd ? (c_t) : (c_f);
		break;
	}

	(r).m_id = 0u;
	(r).m_lit.m_wrd = 0u;

	{
		aclspv_id_t	L$$T = (c_t).m_id, L$$F = (c_f).m_id, L$$v0, L$$C_bool;
		aclspv_wrd_t	L$$arr[7];

		L$$v0	=  aclval_cfg_by_enum(0u, h_ctx, (enum ACLSPV_ID_)L$$size_ty);
		if(0 == L$$T) L$$T = aclval_cfg_by_enum((c_t).m_lit.m_wrd2, h_ctx, (enum ACLSPV_ID_)L$$size_ty);
		if(0 == L$$F) L$$F = aclval_cfg_by_enum((c_f).m_lit.m_wrd2, h_ctx, (enum ACLSPV_ID_)L$$size_ty);

		L$$arr[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5u);
		L$$arr[1] = ACLSPV_ID_BOOL;
		L$$arr[2] = 0u;
		L$$arr[3] = SpvOpINotEqual;
		L$$arr[4] = L$$v0;
		L$$arr[5] = (c_cnd).m_id;

		ae2f_ifnezerr_strict((L$$C_bool = aclconst_specop(&L$$arr[0], 6u, (h_ctx))) == 0) {
			_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
			(r).m_id	= 0u;
			(r).m_lit.m_wrd	= 0u;
			break;
		}

		L$$arr[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 6u);
		L$$arr[1] = L$$size_ty;
		L$$arr[2] = 0u;
		L$$arr[3] = SpvOpSelect;
		L$$arr[4] = L$$C_bool;
		L$$arr[5] = L$$T;
		L$$arr[6] = L$$F;

		ae2f_ifnezerr_strict(((r).m_id = aclconst_specop(&L$$arr[0], 7u, (h_ctx))) == 0) {
			_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
			(r).m_id	= 0u;
			(r).m_lit.m_wrd	= 0u;
			break;
		}
	}
} while(0); }

ae2f_MAC((L, )) aclconst_specsize_op2_bool(
		aclconst_specsz&	r,
		const aclconst_specsz	c_x,
		const aclconst_specsz	c_y,
		aclspv_opcode_t		c_op,
		aclconst_specsz		(fn_op_lit)(aclconst_specsz& r, const aclconst_specsz c_x, const aclconst_specsz c_y),
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t		L$$size_ty = aclty_mk_default(
			aclty_get_sclr_uint(
				h_ctx->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifneznrm(L$$size_ty) {
		(r).m_id = 0;
		(r).m_lit.m_wrd = 0;
		if((c_x).m_id || (c_y).m_id) { do {
			aclconst_specsz	L$$X = (c_x), L$$Y = (c_y);
			aclspv_id_t	L$$id, L$$v0, L$$v1;
			aclspv_wrd_t	L$$arr[7];

			unless((c_x).m_id) (L$$X).m_id = aclval_cfg_by_enum(
					(L$$X).m_lit.m_wrd2
					, (h_ctx)
					, (enum ACLSPV_ID_)L$$size_ty
					);

			unless((c_y).m_id) (L$$Y).m_id = aclval_cfg_by_enum(
					(L$$Y).m_lit.m_wrd2
					, (h_ctx)
					, (enum ACLSPV_ID_)L$$size_ty
					);

			L$$v0 = aclval_cfg_by_enum(0, h_ctx, (enum ACLSPV_ID_)L$$size_ty);
			L$$v1 = aclval_cfg_by_enum(1, h_ctx, (enum ACLSPV_ID_)L$$size_ty);

			ae2f_ifezerr(aclty_mk_default(ACLSPV_ID_BOOL, h_ctx))
				break;

			ae2f_ifezerr((L$$X).m_id && L$$Y.m_id && L$$v0 && L$$v1)
			{ _aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED); break; }

			L$$arr[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
			L$$arr[1] = ACLSPV_ID_BOOL;
			L$$arr[2] = 0;
			L$$arr[3] = (c_op);
			L$$arr[4] = (L$$X).m_id;
			L$$arr[5] = (L$$Y).m_id;

			ae2f_ifezerr(L$$id = aclconst_specop(&L$$arr[0], 6, h_ctx)) {
				_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
				(r).m_id	= 0;
				(r).m_lit.m_wrd	= 0;
				break;
			}

			L$$arr[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 6);
			L$$arr[1] = L$$size_ty;
			L$$arr[2] = 0;
			L$$arr[3] = SpvOpSelect;
			L$$arr[4] = L$$id;
			L$$arr[5] = L$$v1;
			L$$arr[6] = L$$v0;

			ae2f_ifezerr((r).m_id = aclconst_specop(&L$$arr[0], 7, h_ctx)) {
				_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
				(r).m_id	= 0;
				(r).m_lit.m_wrd	= 0;
				break;
			}
		} while(0); } else {
			(r).m_id = 0;
			fn_op_lit(r, c_x, c_y);
		}
	} else {
		_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
		(r).m_id	= 0;
		(r).m_lit.m_wrd	= 0;
	}
}

ae2f_MAC((L, )) aclconst_specsize_op2(
		aclconst_specsz&	r,
		const aclconst_specsz	c_x,
		const aclconst_specsz	c_y,
		aclspv_opcode_t		c_op,
		aclconst_specsz		(fn_op_lit)(aclconst_specsz& r, const aclconst_specsz c_x, const aclconst_specsz c_y),
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t		L$$size_ty = aclty_mk_default(
			aclty_get_sclr_uint(
				(h_ctx)->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifneznrm(L$$size_ty) {
		(r).m_id = 0;
		(r).m_lit.m_wrd = 0;
		if((c_x).m_id || (c_y).m_id) { do {
			aclconst_specsz	L$$X = (c_x), L$$Y = (c_y);
			aclspv_wrd_t	L$$arr[6];

			unless((c_x).m_id) (L$$X).m_id = aclval_cfg_by_enum(
					(L$$X).m_lit.m_wrd2
					, (h_ctx)
					, (enum ACLSPV_ID_)L$$size_ty
					);

			unless((c_y).m_id) (L$$Y).m_id = aclval_cfg_by_enum(
					(L$$Y).m_lit.m_wrd2
					, (h_ctx)
					, (enum ACLSPV_ID_)L$$size_ty
					);

			ae2f_ifezerr((L$$X).m_id && L$$Y.m_id)
			{ _aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED); break; }

			L$$arr[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
			L$$arr[1] = L$$size_ty;
			L$$arr[2] = 0;
			L$$arr[3] = (c_op);
			L$$arr[4] = (L$$X).m_id;
			L$$arr[5] = (L$$Y).m_id;

			ae2f_ifezerr((r).m_id = aclconst_specop(&L$$arr[0], 6, h_ctx)) {
				_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
				(r).m_id	= 0;
				(r).m_lit.m_wrd	= 0;
				break;
			}

		} while(0); } else {
			(r).m_id = 0;
			fn_op_lit(r, c_x, c_y);
		}
	} else {
		(r).m_id	= 0;
		(r).m_lit.m_wrd	= 0;
	}
}

ae2f_MAC((L, )) aclconst_specsize_op1(
		aclconst_specsz&	r,
		const aclconst_specsz	c,
		aclspv_opcode_t		c_op,
		aclconst_specsz		(fn_op_lit)(aclconst_specsz& r, const aclconst_specsz c),
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t		L$$size_ty = aclty_mk_default(
			aclty_get_sclr_uint(
				(h_ctx)->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifneznrm(L$$size_ty) {
		(r).m_id = 0;
		(r).m_lit.m_wrd = 0;
		if((c).m_id) { do {
			aclconst_specsz	L$$X = (c);
			aclspv_wrd_t	L$$arr[5];

			unless((c).m_id) (L$$X).m_id = aclval_cfg_by_enum(
					(L$$X).m_lit.m_max
					, (h_ctx)
					, (enum ACLSPV_ID_)L$$size_ty
					);

			ae2f_ifezerr((L$$X).m_id)
			{ _aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED); break; }

			L$$arr[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 4);
			L$$arr[1] = L$$size_ty;
			L$$arr[2] = 0;
			L$$arr[3] = (c_op);
			L$$arr[4] = (L$$X).m_id;

			ae2f_ifezerr((r).m_id = aclconst_specop(&L$$arr[0], 5, h_ctx)) {
				_aclctx_err((h_ctx), ACLSPV_COMPILE_ALLOC_FAILED);
				(r).m_id	= 0;
				(r).m_lit.m_wrd	= 0;
				break;
			}
		} while(0); } else {
			(r).m_id = 0;
			fn_op_lit(r, c);
		}
	} else {
		(r).m_id	= 0;
		(r).m_lit.m_wrd	= 0;
	}
}

#endif
