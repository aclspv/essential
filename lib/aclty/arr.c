#include "../aclty.h"
#include "./arr.h"
#include "./implutil.h"

#include <aclmem.h>
#include <aclexpr.h>
#include <aclctx/err.h>

#include <util/emitx.h>
#include <util/iddef.h>
#include <aclutil/capability.h>

#include <ae2f/pat/dbg.h>

#include <.aclspv/const/spec.h>

aclspv_id_t aclty_mk_arr3(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		aclspv_wrdcount_t	c_stride,
		const h_aclspv_ctx_t	h_ctx
		) {
	typedef aclty_t* ae2f_restrict p_cxtp_t;
	p_cxtp_t arr = aclty_cfg_arr(c_type_elm, c_num_arr, h_ctx), r;
	ae2f_ifezerr(arr) {
		err_puts("aclutil_cfg_cxtp_arr has failed");
		return 0;
	}

	if(c_stride != 0xFFFFFFFF) {
		p_cxtp_t arr_stride = aclty_cfg_arr_dcr(
				arr
				, SpvDecorationArrayStride
				, h_ctx
				);

		aclspv_wrd_t stride;
		aclconst_specsz elm_size = aclty_getsize(c_type_elm, h_ctx);

		ae2f_ifnezerr(elm_size.m_id) {
			err_puts("it has elm_size with id\n");
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 0;
		}

		stride = ae2f_expected_not_strict(elm_size.m_lit.m_max > (aclspv_wrdmax_t)0xFFFFFFFF)
			? 0
			: elm_size.m_lit.m_wrd;

		ae2f_ifezerr(stride) {
			return 0;
		}
		stride = (stride + c_stride - 1) & ~(c_stride - 1);

		arr_stride->m_info.m_dcr.m_num_oprnd = 1;
		arr_stride->m_info.m_dcr.m_oprnd[0] = stride;
	}

	r = aclty_build_arr(arr, h_ctx);
	ae2f_ifezerr(r) {
		err_puts("aclty_build_arr has failed\n");
		return 0;
	}

	ae2fpat_assertassume_strict(r->m_type_id);

	return r->m_type_id;
}

aclspv_id_t aclty_mk_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrd_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz num_arr;
	num_arr.m_id = c_num_arr;
	num_arr.m_lit.m_max = 0;

	return aclty_mk_arr3(
			c_type_elm
			, num_arr
			, c_decorate_stride
			, h_ctx
			);
}

#include <aclconst/align.h>
#include "./dcr.h"
#include "./dcr/util.h"

#if	1

static aclspv_bool_t s_is_same(
		const aclty_t* ae2f_restrict	rd0,
		const aclty_t* ae2f_restrict	rd1,
		h_aclspv_ctx_t				h_ctx
		) {
	if(rd0 == rd1) return 0;
	if(rd0->m_class != rd1->m_class) return 0;
	if(rd0->m_alignof != rd1->m_alignof) return 0;
	if(rd0->m_info.m_arr.m_type != rd1->m_info.m_arr.m_type) return 0;
	unless(aclconst_is_specsz_same(rd0->m_sizeof, rd1->m_sizeof, h_ctx))

	if(rd0->m_info.m_arr.m_num_dcr != rd1->m_info.m_arr.m_num_dcr)
		return 0;

	if(rd0->m_info.m_arr.m_num.m_id != rd1->m_info.m_arr.m_num.m_id)
		return 0;

	unless(rd0->m_info.m_arr.m_num_dcr)
		return 1;

	return aclty_is_dcr_same(
			&__get_type_uniques(h_ctx)[rd0->m_info.m_arr.m_dcr]
			, &__get_type_uniques(h_ctx)[rd1->m_info.m_arr.m_dcr]	
			, h_ctx
			);
}

static void s_free(aclty_t* ae2f_restrict fini, h_aclspv_ctx_t h_ctx) {
	if(fini->m_info.m_arr.m_dcr) {
		aclty_free_dcr(&__get_type_uniques(h_ctx)[fini->m_info.m_arr.m_dcr], h_ctx);
	}

	fini->m_class = ACLTY_RESERVED;
}

aclty_t* aclty_build_arr(
		aclty_t* ae2f_restrict	rdwr_arr,
		const h_aclspv_ctx_t		h_ctx
		) {
	aclspv_wrd_t	i = h_ctx->m_num_type_uniques;

	ae2f_ifezerr(rdwr_arr) return 0;
	ae2f_ifnezerr_strict(rdwr_arr->m_class != ACLTY_ARR)
		return 0;

	while(i --> 0) {
		aclty_t* cxtp2 = &__get_type_uniques(h_ctx)[i];
		if(s_is_same(rdwr_arr, cxtp2, h_ctx)) {
			/** failed */
			s_free(rdwr_arr, h_ctx);
			return cxtp2;
		}
	}

	if(!rdwr_arr->m_info.m_arr.m_num.m_id && rdwr_arr->m_info.m_arr.m_num.m_lit.m_max) {
		const aclspv_id_t SIZE_TY = aclid_getty_uint(h_ctx->m_usrcfg.m_byte_size_type << 3);
		ae2f_ifezerr(SIZE_TY)	return 0;

		ae2f_ifezerr(rdwr_arr->m_info.m_arr.m_num.m_id = aclval_cfg_by_enum(
					rdwr_arr->m_info.m_arr.m_num.m_lit.m_max
					, h_ctx
					, (enum ACLSPV_ID_)SIZE_TY))
			return 0;
	} else {
		rdwr_arr->m_info.m_arr.m_num.m_lit.m_max = 0;
	}

	if(rdwr_arr->m_info.m_arr.m_num.m_id) {
		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeArray
					, rdwr_arr->m_type_id
					, rdwr_arr->m_info.m_arr.m_type
					, rdwr_arr->m_info.m_arr.m_num.m_id
					)) return 0;
	} else {
#if	__ACLSPV_OpCapabilityShader
		ae2f_ifezerr(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityShader)) {
			err_puts("SpvOpTypeRuntimeArray, SpvCapabilityShader");
			return 0;
		}

		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_3(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeRuntimeArray
					, rdwr_arr->m_type_id
					, rdwr_arr->m_info.m_arr.m_type
					)) return 0;

		rdwr_arr->m_field0 |= SpvOpTypeRuntimeArray;
#else
		return 0;
#endif
	}

	ae2f_ifnezerr_strict(rdwr_arr->m_info.m_arr.m_dcr && aclty_build_dcr(
				&__get_type_uniques(h_ctx)[rdwr_arr->m_info.m_arr.m_dcr]
				, rdwr_arr->m_type_id
				, h_ctx
				)) return 0;

	return rdwr_arr;
}

aclty_t* aclty_cfg_arr_dcr(
		aclty_t* ae2f_restrict	rdwr_arr,
		const enum SpvDecoration_	c_dcr,
		const h_aclspv_ctx_t		h_ctx
		) {
	aclty_t* ae2f_restrict r;

	ae2f_ifezerr(rdwr_arr) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	ae2f_ifnezerr_strict(rdwr_arr->m_class != ACLTY_ARR) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_TYINVL_ARR_DCR;
		return 0;
	}

	if(rdwr_arr->m_info.m_arr.m_dcr) {
		aclty_t* g;
		if((g = aclty_get_dcr(
						&__get_type_uniques(h_ctx)[rdwr_arr->m_info.m_arr.m_dcr]
						, (aclspv_wrd_t)c_dcr
						, h_ctx)))
			return g;
	}

	{
		struct __ret ID;
		aclty_t TAR;
		aclmemzero(&TAR, sizeof(TAR));

		TAR.m_class	= ACLTY_DCR;
		TAR.m_info.m_dcr.m_op		= (aclspv_wrd_t)c_dcr;
		TAR.m_info.m_dcr.m_nxt		= 0;

		ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++, 1);
		unless(ID.m_id)	return 0;

		r = aclty_find(ID.m_id, h_ctx);
	}

	ae2f_ifezerr(r)	return 0;

	++rdwr_arr->m_info.m_arr.m_num_dcr;
	unless(rdwr_arr->m_info.m_arr.m_dcr) {
		rdwr_arr->m_info.m_arr.m_dcr = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));
		return r;
	}

	return s_dcr_insert_invoke(
			(aclspv_wrd_t)c_dcr
			, r
			, &(__get_type_uniques(h_ctx)[rdwr_arr->m_info.m_arr.m_dcr])
			, h_ctx
			);
}

aclty_t* aclty_cfg_arr(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclty_t            tar;
	aclconst_specsz		size_elm;
	aclspv_id_t		size_ty = aclid_mk_default_id(
			aclid_getty_uint(
				h_ctx->m_usrcfg.m_byte_size_type << 3
				)
			, h_ctx
			);

	aclmemzero(&tar, sizeof(tar));

	ae2f_ifezerr(size_ty) {
		err_puts("size_ty is 0");
		return 0;
	}

	if(c_num_arr.m_id) {
		tar.m_info.m_arr.m_num.m_id		= c_num_arr.m_id;
	} else ae2f_ifneznrm(c_num_arr.m_lit.m_max) {
		ae2f_ifezerr(tar.m_info.m_arr.m_num.m_id = aclval_cfg_by_enum(
					c_num_arr.m_lit.m_max
					, h_ctx
					, (enum ACLSPV_ID_)size_ty)) return 0;
	}	/** trust it is going to be runtime array */ 

	tar.m_class		= ACLTY_ARR;
	tar.m_info.m_arr.m_type	= c_type_elm;
	tar.m_alignof		= aclty_getalign(c_type_elm, h_ctx);

	size_elm = aclconst_alignup1(
			aclty_getsize(c_type_elm, h_ctx)
			, tar.m_alignof
			, h_ctx
			);

#define	__MUL(r, c_x, c_y)							\
	{									\
		(r).m_id = 0;							\
		(r).m_lit.m_max = (c_x).m_lit.m_max * (c_y).m_lit.m_max;	\
	}

	_aclconst_specsize_op2(
			L
			, tar.m_sizeof
			, size_elm
			, c_num_arr
			, SpvOpIMul
			, __MUL
			, h_ctx
			);
#undef	__MUL

	{
		const struct __ret ID = s_ret_new(h_ctx, &tar, 0, 1);
		ae2f_ifezerr(ID.m_id)		return 0;

		return aclty_find(ID.m_id, h_ctx);
	}
}
#endif
