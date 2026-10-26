/** 
 * @file repvec.c 
 * */

#include <aclval.h>
#include <aclty.h>
#include <aclcur/opX.h>
#include <util/wrdemit.h>
#include <util/literal.h>

#include <ae2f/pat/dbg.h>

ae2f_inline_f static struct aclutil_mk_constant_repvec_t ae2f_ccconst s_mkret(const aclspv_wrd_t c_id, const aclspv_wrd_t c_mask) {
	struct aclutil_mk_constant_repvec_t R;
	R.m_id = c_id;
	R.m_literal_mask = c_mask;
	return R;
}

struct aclutil_mk_constant_repvec_t	aclval_cfg_vec_broadcast(
		const aclspv_id_t	c_id_ty,
		const aclspv_id_t c_val,
		const aclspv_wrd_t c_literal,
		h_aclspv_ctx_t h_ctx) {
	const u64_fast KEY = (u64_fast)(c_id_ty) | (((u64_fast)(c_val)) << (u64_fast)32);
	aclval_t* C = aclval_cfg_node(KEY, h_ctx);

	aclspv_id_t	VEC_COUNT = aclty_get_veccfg(c_id_ty, h_ctx).m_num;

	const aclspv_wrd_t	OPCODE_PURE = ae2f_bll_sel_bool_naive1(
			aclspv_wrd_t
			, c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)
			, SpvOpSpecConstantComposite
			, SpvOpCompositeConstruct);

	const aclspv_wrd_t	OPCODE = aclspv_opcode_to_wrd(OPCODE_PURE, VEC_COUNT + 2);


	x_aclspv_vec* ae2f_restrict const P_VEC =
			c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)
			? &h_ctx->m_section.m_types
			: &h_ctx->m_section.m_fnimpl;

	aclspv_wrd_t* ae2f_restrict const P_VWRD =
			c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)
			? &h_ctx->m_count.m_types
			: &h_ctx->m_count.m_fnimpl;

	const aclspv_wrd_t	ANCHOUR = *P_VWRD;

	dbg_puts("aclutil_mk_constant_repvec_id");

	ae2f_ifezerr(C) {
		h_ctx->m_id = ACLSPV_COMPILE_ALLOC_FAILED_VEC;
		return s_mkret(0, 0);
	}

	switch(VEC_COUNT) {
		case 0: return s_mkret(0, 0);
		case 1: return s_mkret(c_val, c_literal);
		default: break;
	}

	if(C->m_repvec)
		return s_mkret(C->m_repvec, C->m_repvec_literal_mask);

	{
		/** all operands are the same id: pre-fill the operand list. */
		aclspv_id_t os[16];
		aclspv_wrd_t i;

		ae2f_ifnezerr_strict(VEC_COUNT > 16) {
#if	__ACLSPV_CHECK_EXTRA
			err_puts("repvec: VEC_COUNT exceeds 16.");
#endif
			return s_mkret(0, 0);
		}

		for(i = 0; i < VEC_COUNT; ++i)
			os[i] = c_val;

		if(c_literal & (ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL)) {
			/** all-constant path: emit directly into m_types. */
			ae2f_ifezerr(*P_VWRD = aclutil_emit_wrd(P_VEC, *P_VWRD + 2 + VEC_COUNT, 0)) {
				h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_VEC;
				return s_mkret(0, 0);
			}

			ae2fpat_assertassume(aclutil_get_wrd_of_vec(P_VEC));

			aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 0] = OPCODE;
			aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 1] = c_id_ty;
			aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 2] = C->m_repvec = h_ctx->m_id++;

			while(VEC_COUNT --> 0) {
				aclutil_get_wrd_of_vec(P_VEC)[ANCHOUR + 3 + VEC_COUNT] = c_val;
			}

			C->m_repvec_literal_mask = c_literal & ~(aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL;

			if(c_literal & ACLUTIL_LITERAL_LITERAL) {
				C->m_repvec_literal_mask |= ACLUTIL_LITERAL_CONSTANT;
			}

			return s_mkret(
					C->m_repvec
					, C->m_repvec_literal_mask
				      );
		}

		/** runtime operands: build an opX node so aclcur_emit writes
		 *  OpCompositeConstruct into m_fnimpl in program order. */
		{
			aclspv_id_t OPC = aclcur_cfg_opx(
					h_ctx->m_id++
					, SpvOpCompositeConstruct
					, c_id_ty
					, os
					, VEC_COUNT
					, h_ctx
					);
			ae2f_ifezerr(OPC) {
				h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
				return s_mkret(0, 0);
			}
			C->m_repvec = OPC;
			C->m_repvec_literal_mask = c_literal & ~(aclspv_wrd_t)(ACLUTIL_LITERAL_CONSTANT | ACLUTIL_LITERAL_LITERAL);
			return s_mkret(OPC, C->m_repvec_literal_mask);
		}
	}
}
