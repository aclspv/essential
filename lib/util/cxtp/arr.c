#include "./arr.h"
#include "./implutil.h"
#include "./sizeof.h"

#include <util/emitx.h>
#include <util/iddef.h>
#include <util/constant.h>

#include <ae2f/cc/memset.h>
#include <.aclspv/const/spec.h>

static aclspv_id_t s_arr(
		aclutil_cxtp c_tar,
		const aclspv_id_t c_num_arr,
		const aclspv_id_t c_type_elm,
		const aclspv_wrd_t	c_decorate_stride,
		h_aclspv_ctx_t		h_ctx
		) {
	dbg_prefix();
	dbg_puts("s_arr\n");

	if(c_decorate_stride ^ 0xFFFFFFFF) {
		c_tar.m_info.m_arr.m_decorate_stride	= c_decorate_stride;
	} else {
		c_tar.m_info.m_arr.m_decorate_stride	= c_tar.m_alignof;
	}

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &c_tar);
		ae2f_ifezerr(ID.m_id)		return 0;
		unless(ID.m_isfirst)            return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeArray
					, ID.m_id
					, c_type_elm
					, c_num_arr
					)) return 0;

		ae2f_expected_but_else(h_ctx->m_count.m_decorate = aclutil_emitx_4(
					&h_ctx->m_section.m_decorate
					, h_ctx->m_count.m_decorate
					, SpvOpDecorate
					, ID.m_id
					, SpvDecorationArrayStride
					, c_decorate_stride
					)) return 0;

		return ID.m_id;
	}
}


aclspv_id_t aclutil_mk_cxtp_arr3(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		const aclspv_wrdcount_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclutil_cxtp            TAR;
	aclspv_id_t		size_elm;
	aclspv_id_t		size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				h_ctx->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	ae2fcc_memset(&TAR, 0, sizeof(TAR));

	ae2f_ifezerr(size_ty) {
		return 0;
	}

	TAR.m_class = ACLUTIL_CXTYPE_ARRAY;

	if(c_num_arr.m_id) {
		TAR.m_info.m_arr.m_num_id		= c_num_arr.m_id;
	} else ae2f_ifneznrm(c_num_arr.m_lit.m_wrd) {
		ae2f_ifezerr(TAR.m_info.m_arr.m_num_id = aclutil_mk_constant_val_by_enum(
				c_num_arr.m_lit.m_wrd
				, h_ctx
				, (aclspv_id_t)size_ty)) return 0;
	} else {
		return 0;
	}

	TAR.m_info.m_arr.m_type			= c_type_elm;
	TAR.m_alignof	= aclutil_cxtp_alignof(c_type_elm, h_ctx);

	if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_ISOC_MATCH_SIZE_AND_ALIGNMENT) {
		/** sizeof(T) will therefore be alignof(T) */
		size_elm = aclutil_cxtp_sizeof(c_type_elm, h_ctx);
	} else {
		size_elm = aclutil_mk_constant_val_by_enum(
				TAR.m_alignof
				, h_ctx
				, size_ty);
	}

	{
#define	__MUL(r, c_x, c_y)								\
		{									\
			(r).m_id = 0;							\
			(r).m_lit.m_wrd = (c_x).m_lit.m_wrd * (c_y).m_lit.m_wrd;	\
		}

		aclconst_specsz x;

		x.m_id	= 0;
		x.m_lit.m_wrd = size_elm;

		_aclconst_specsize_op2(
				L
				, TAR.m_sizeof
				, x
				, c_num_arr
				, SpvOpIMul
				, __MUL
				, h_ctx
				);
#undef	__MUL
	}

	return s_arr(
			TAR
			, TAR.m_info.m_arr.m_num_id
			, c_type_elm
			, c_decorate_stride
			, h_ctx
			);
}



aclspv_id_t aclutil_mk_cxtp_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrd_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclutil_cxtp            TAR;
	aclspv_id_t		size_elm;
	aclspv_id_t		size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				h_ctx->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	dbg_puts("aclutil_mk_cxtp_arr2");

	ae2fcc_memset(&TAR, 0, sizeof(TAR));

	ae2f_ifezerr(size_ty) {
		err_puts("size_ty is 0");
		return 0;
	}

	TAR.m_class = ACLUTIL_CXTYPE_ARRAY;
	TAR.m_info.m_arr.m_num_id		= c_num_arr;
	TAR.m_info.m_arr.m_type			= c_type_elm;
	TAR.m_alignof	= aclutil_cxtp_alignof(c_type_elm, h_ctx);

	if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_ISOC_MATCH_SIZE_AND_ALIGNMENT) {
		/** sizeof(T) will therefore be alignof(T) */
		size_elm = aclutil_cxtp_sizeof(c_type_elm, h_ctx);
	} else {
		size_elm = aclutil_mk_constant_val_by_enum(
				TAR.m_alignof
				, h_ctx
				, size_ty);
	}

	{
#define	__MUL(r, c_x, c_y)								\
		{									\
			(r).m_id = 0;							\
			(r).m_lit.m_wrd = (c_x).m_lit.m_wrd * (c_y).m_lit.m_wrd;	\
		}

		aclconst_specsz x, y;

		x.m_id	= 0;
		x.m_lit.m_wrd = size_elm;

		y.m_id	= c_num_arr;
		y.m_lit.m_wrd = 0;

		_aclconst_specsize_op2(
				L
				, TAR.m_sizeof
				, x
				, y
				, SpvOpIMul
				, __MUL
				, h_ctx
				);
#undef	__MUL
	}

	return s_arr(
			TAR
			, TAR.m_info.m_arr.m_num_id
			, c_type_elm
			, c_decorate_stride
			, h_ctx
			);
}
