#ifndef aclspv_const_spec_cc
#define aclspv_const_spec_cc

#include <aclconst/specsize.h>
#include <ae2f/pat/macrofunc.h>
#include <util/wrd.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <util/constant.h>

ae2f_MAC((L, )) aclconst_specsize_op3_sel(
		aclconst_specsz&	r,
		const aclconst_specsz	c_cnd,
		const aclconst_specsz	c_t,
		const aclconst_specsz	c_f,
		h_aclspv_ctx_t		h_ctx
		) { do {
	aclspv_id_t		L$$size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				h_ctx->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifezerr(L$$size_ty) {
		(h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; 
		(r).m_id	= 0;
		(r).m_lit.m_wrd	= 0;
		break;
	}

	unless((c_cnd).m_id) {
		(r) = (c_cnd).m_lit.m_wrd ? c_t : c_f;
		break;
	}

	(r).m_id = 0;
	(r).m_lit.m_wrd = 0;

	{
		aclspv_id_t	L$$T = (c_t).m_id, L$$F = (c_f).m_id, L$$v0, L$$C_bool;

		L$$v0	= aclutil_mk_constant_val_by_enum(0, h_ctx, L$$size_ty);
		unless(L$$T) L$$T = aclutil_mk_constant_val_by_enum((c_t).m_lit.m_wrd2, h_ctx, L$$size_ty);
		unless(L$$F) L$$F = aclutil_mk_constant_val_by_enum((c_f).m_lit.m_wrd2, h_ctx, L$$size_ty);

		ae2f_ifezerr((h_ctx)->m_count.m_types = aclutil_emitx_6(
					&(h_ctx)->m_section.m_types
					, (h_ctx)->m_count.m_types
					, SpvOpSpecConstantOp
					, ACLSPV_ID_BOOL
					, L$$C_bool = (h_ctx)->m_id++
					, SpvOpINotEqual
					, L$$v0
					, (c_cnd).m_id
					)) {
			(h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break;
			(r).m_id	= 0;
			(r).m_lit.m_wrd	= 0;
		}

		ae2f_ifezerr((h_ctx)->m_count.m_types = aclutil_emitx_7(
					&(h_ctx)->m_section.m_types
					, (h_ctx)->m_count.m_types
					, SpvOpSpecConstantOp
					, L$$size_ty
					, (r).m_id = (h_ctx)->m_id++
					, SpvOpSelect
					, L$$C_bool
					, L$$T
					, L$$F
					)) {
			(h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break;
			(r).m_id	= 0;
			(r).m_lit.m_wrd	= 0;
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
	aclspv_id_t		L$$size_ty = aclid_mk_default_id(
			aclid_getty_uint(
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

			L$$v0 = aclutil_mk_constant_val_by_enum(0, h_ctx, L$$size_ty);
			L$$v1 = aclutil_mk_constant_val_by_enum(1, h_ctx, L$$size_ty);

			ae2f_ifezerr(aclid_mk_default_id(ACLSPV_ID_BOOL, h_ctx))
				break;

			ae2f_ifezerr((L$$X).m_id && L$$Y.m_id && L$$v0 && L$$v1)
			{ (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }

			ae2f_ifezerr((h_ctx)->m_count.m_types = aclutil_emitx_6(
						&(h_ctx)->m_section.m_types
						, (h_ctx)->m_count.m_types
						, SpvOpSpecConstantOp
						, ACLSPV_ID_BOOL
						, L$$id = (h_ctx)->m_id++
						, (c_op)
						, (L$$X).m_id
						, (L$$Y).m_id
						)) { (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }

			ae2f_ifezerr((h_ctx)->m_count.m_types = aclutil_emitx_7(
						&(h_ctx)->m_section.m_types
						, (h_ctx)->m_count.m_types
						, SpvOpSpecConstantOp
						, L$$size_ty
						, (r).m_id = (h_ctx)->m_id++
						, SpvOpSelect
						, L$$id
						, L$$v1
						, L$$v0
						)) { (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }
		} while(0); } else {
			(r).m_id = 0;
			fn_op_lit(r, c_x, c_y);
		}
	} else {
		(h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
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
	aclspv_id_t		L$$size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				(h_ctx)->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifneznrm(L$$size_ty) {
		(r).m_id = 0;
		(r).m_lit.m_wrd = 0;
		if((c_x).m_id || (c_y).m_id) { do {
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

			ae2f_ifezerr((h_ctx)->m_count.m_types = aclutil_emitx_6(
						&(h_ctx)->m_section.m_types
						, (h_ctx)->m_count.m_types
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

ae2f_MAC((L, )) aclconst_specsize_op1(
		aclconst_specsz&	r,
		const aclconst_specsz	c,
		aclspv_opcode_t		c_op,
		aclconst_specsz		(fn_op_lit)(aclconst_specsz& r, const aclconst_specsz c),
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t		L$$size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				(h_ctx)->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2f_ifneznrm(L$$size_ty) {
		(r).m_id = 0;
		(r).m_lit.m_wrd = 0;
		if((c).m_id) { do {
			aclconst_specsz	L$$X = (c);

			unless((c).m_id) (L$$X).m_id = aclutil_mk_constant_val_by_enum(
					(L$$X).m_lit.m_max
					, (h_ctx)
					, L$$size_ty
					);

			ae2f_ifezerr((L$$X).m_id)
			{ (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }

			ae2f_ifezerr((h_ctx)->m_count.m_types = aclutil_emitx_5(
						&(h_ctx)->m_section.m_types
						, (h_ctx)->m_count.m_types
						, SpvOpSpecConstantOp
						, L$$size_ty
						, (r).m_id = (h_ctx)->m_id++
						, (c_op)
						, (L$$X).m_id
						)) { (h_ctx)->m_err = ACLSPV_COMPILE_ALLOC_FAILED; break; }
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
