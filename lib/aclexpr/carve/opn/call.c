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

#include "./call.h"
#include <aclemit/inline.h>
#include <aclctx/err.h>

#include <aclsym/fn.h>

#include <aclexpr/carve/util.h>

#include <aclcur/common.h>
#include <aclcur/opX.h>
#include <aclcur/reg.h>
#include <aclcur/load.h>
#include <aclcur/store.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <ae2f/pat/dbg.h>

/** a call may have written state this function does not own: it names no var
 * of ours, so it bumps the alias generation and every cached value that is
 * only current while nothing unknown was written reloads on its next read.
 * an inlined body's stores go through the ordinary store path instead. */
static void s_clobber_reachable(h_aclspv_ctx_t h_ctx) {
	aclcur_reg_gen_wrote(0, h_ctx);
}

aclspv_bool_t aclexpr_call_make(
		aclspv_id_t		c_fn,
		aclty_id_t		c_ty_ret,
		const aclspv_vid_t	c_args[],
		aclspv_wrd_t		c_num,
		aclspv_id_t* ae2f_restrict	wr_id,
		aclspv_bool_t* ae2f_restrict	wr_is_pure,
		h_aclspv_ctx_t		h_ctx
		) {
	aclsym_fn_t* ae2f_restrict p_sym;
	aclspv_bool_t			is_pure = 0;

	ae2f_ifezerr(*wr_id = aclcur_cfg_call(
				c_fn, c_ty_ret, c_args, c_num, h_ctx))
		return 1;

	p_sym = aclsym_fn_find_id(c_fn, h_ctx);

	/** the callee's promise is the call site's effect, and it propagates:
	 * a caller of a non-const callee may read external state, a caller of a
	 * non-pure callee may write it. an unknown callee promises nothing.
	 * CONST alone must not keep the clobbered caches alive (a const
	 * function may still write), but it does describe the reads. */
	{
		h_aclcur_t p_call = aclcur_get_node(*wr_id, h_ctx);

		if(p_call) {
			if(!p_sym || !(p_sym->m_flag0
						& aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST)))
				p_call->m_flag0 |= aclcur_flag0(
						ACLCUR_FLAG0_CALL_NONCONST);

			if(!p_sym || !(p_sym->m_flag0
						& aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE)))
				p_call->m_flag0 |= aclcur_flag0(
						ACLCUR_FLAG0_CALL_NONPURE);
		}
	}

	/** only PURE says "no writes". */
	if(p_sym)
		is_pure = (aclspv_bool_t)!!(p_sym->m_flag0
				& aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE));

	/** unknown callee counts as impure: forget what memory may have held. */
	unless(is_pure)
		s_clobber_reachable(h_ctx);

	if(wr_is_pure)
		*wr_is_pure = is_pure;

	return 0;
}

/** a pointer argument must reach OpFunctionCall as the memory object itself.
 * address-of wraps the storage id in OpCopyObject so the rest of the
 * expression machinery sees a typed value; the call must see the variable.
 *
 * a pointer into StorageBuffer / Uniform / ... needs SPIR-V variable
 * pointers; aclmit_fndecl_norm requests the capability when it emits a
 * function with such a parameter. */
static aclspv_id_t s_arg_value(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p = aclcur_get_node(c_id, h_ctx);

	if(!p || p->m_class != ACLCUR_OP1)
		return c_id;
	if(p->m_d.m_op.m_op != SpvOpCopyObject)
		return c_id;

	{
		aclty_t* ae2f_restrict p_ty = aclty_find(p->m_ty, h_ctx);

		if(p_ty && p_ty->m_class == ACLTY_PTR)
			return p->m_d.m_op.m_o[0];
	}

	return c_id;
}

aclspv_bool_t aclexpr_carve_opn_call(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		acltrl_t* ae2f_restrict			r_ltrl
		) {
	h_aclspv_ctx_t		ctx = CTX(rdwr_data);
	const aclspv_wrd_t	c_num = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_NUMOPRND];
	const aclspv_id_t	c_fn = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_SPECCONSTANTOP];
	const aclspv_id_t	c_ty_fn = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID];
	const aclty_id_t	c_ty_ret = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	/** the call operands: a pointer argument is the memory object itself, not
	 * the OpCopyObject wrapping its address (see s_arg_value). */
	aclspv_vid_t		os[__ACLSPV_MAX_OPRND];
	/** the value form of the same operands: a pointer argument stays the typed
	 * pointer VALUE. an inline binds it as a ghost over that value, so
	 * unwrapping here would hand aclemit_inline_fn a storage id whose type is
	 * the pointee, not a pointer. */
	aclspv_vid_t		vs[__ACLSPV_MAX_OPRND];
	aclspv_bool_t		is_pure = 0;
	aclty_t* ae2f_restrict	p_fn;
	aclspv_wrd_t		i;

	ae2f_ifezerr(p_fn = aclty_find(c_ty_fn, ctx))
		return 1;

	if(c_num + 1 > __ACLSPV_MAX_OPRND)
		return 1;

	for(i = 0; i < c_num; ++i) {
		const aclspv_id_t	c_id = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + i];
		const aclty_id_t	c_ty_from = rdwr_lst_extra[i].m_type_original;
		aclty_t* ae2f_restrict	p_arg;
		aclty_id_t		c_ty_to;

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_fn, i, ctx))
			return 1;

		c_ty_to = p_arg->m_info.m_fn_arg.m_prm_id;

		if(c_ty_from == c_ty_to) {
			vs[i + 1] = c_id;
		}
		else {
			acltrl_t		lit = rdwr_lst_extra[i].m_literal;
			enum ACLEXPR_RET_	R;

			R = aclexpr_arithmetic_cast(
					c_id
					, c_ty_from
					, rdwr_lst_extra[i].m_mask_literal
					, lit
					, &vs[i + 1]
					, c_ty_to
					, 0
					, ctx
					);

			switch(R) {
				default:
				case ACLEXPR_RET_FAILURE:
				case ACLEXPR_RET_NOT_THE_CASE:
					err_puts("[carve::call] argument cast failed");
					return 1;

				case ACLEXPR_RET_SUCCESS:
				case ACLEXPR_RET_SUCCESS_CONSTANT:
				case ACLEXPR_RET_SUCCESS_LITERAL:
					break;
			}
		}

		os[i + 1] = s_arg_value(vs[i + 1], ctx);
	}

	if(rdwr_lst_extra[c_num].m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_INLINE_CALL) {
		acltrl_mask_t	LITMASK = 0;
		acltrl_t	LIT;

		aclmemzero(&LIT, sizeof(LIT));

		if(!aclemit_inline_fn(
					rdwr_lst_extra[c_num].m_cursor
					, clang_getNullCursor()
					, &vs[1]
					, c_num
					, rdwr_prv_idval
					, &LITMASK
					, &LIT
					, ctx))
			return 1;

		if((LITMASK & ACLTRL_MASK_LTRL) == ACLTRL_MASK_LTRL)
			*r_ltrl = LIT;

		rdwr_prv_extra->m_type_original = c_ty_ret;
		rdwr_prv_extra->m_mask_literal = LITMASK;
		rdwr_prv_extra->m_literal = LIT;

		return 0;
	}

	ae2f_ifnezerr(aclexpr_call_make(
				c_fn, c_ty_ret, &os[1], c_num
				, rdwr_prv_idval, &is_pure, ctx))
		return 1;

	aclmemzero(r_ltrl, sizeof(*r_ltrl));

	rdwr_prv_extra->m_type_original	= c_ty_ret;
	rdwr_prv_extra->m_mask_literal	= ACLTRL_MASK_RT;
	rdwr_prv_extra->m_state0	= is_pure
		? (aclspv_wrd_t)0
		: (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_NOPURE;

	return 0;
}
