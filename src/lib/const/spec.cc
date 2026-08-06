#ifndef aclspv_const_spec_cc
#define aclspv_const_spec_cc

#include <aclconst/specsize.h>
#include <ae2f/pat/macrofunc.h>
#include <util/wrd.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <util/constant.h>

ae2f_MAC((L, )) aclconst_specsize_op2(
		aclconst_specsz&	r,
		const aclconst_specsz	c_x,
		const aclconst_specsz	c_y,
		aclspv_opcode_t		c_op,
		aclconst_specsz		(fn_op_lit)(aclconst_specsz& r, const aclconst_specsz c_x, const aclconst_specsz c_y),
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t		L$$size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				h_ctx->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifneznrm(L$$size_ty) {
		(r).m_id = 0;
		(r).m_lit.m_wrd = 0;
		if(c_x.m_id || c_y.m_id) { do {
			aclconst_specsz	L$$X = (c_x), L$$Y = (c_y);

			unless((c_x).m_id) (L$$X).m_id = aclutil_mk_constant_val_by_enum(
							(L$$X).m_lit.m_wrd2
							, (h_ctx)
							, L$$size_ty
							);

			unless((c_y).m_id) (L$$Y).m_id = aclutil_mk_constant_val_by_enum(
							(L$$Y).m_lit.m_wrd2
							, (h_ctx)
							, L$$size_ty
							);

			ae2f_ifezerr((L$$X).m_id && L$$Y.m_id)
			{ (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }

			ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_6(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstantOp
						, L$$size_ty
						, (r).m_id = (h_ctx)->m_id++
						, (c_op)
						, (L$$X).m_id
						, (L$$Y).m_id
						)) { (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }
		} while(0); } else {
			(r).m_id = 0;
			fn_op_lit(r, c_x, c_y);
		}
	} else {
		(r).m_id	= 0;
		(r).m_lit.m_wrd	= 0;
	}
}

#endif
