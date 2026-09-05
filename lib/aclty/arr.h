#ifndef	aclutil_cxtp_arr_h
#define	aclutil_cxtp_arr_h

#include "./common.h"
#include <.aclspv/const/spec.h>

aclspv_id_t aclty_mk_arr3(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		aclspv_wrdcount_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		);

/** array with stride */
aclspv_id_t aclty_mk_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrdcount_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		);

ae2f_inline_f static aclspv_id_t aclty_mk_arr(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		) {
	return aclty_mk_arr2(c_type_elm, c_num_arr, 0xFFFFFFFF, h_ctx);
}

aclty_t* aclty_cfg_arr_dcr(
		aclty_t* ae2f_restrict	rdwr_arr,
		const enum SpvDecoration_	c_dcr,
		const h_aclspv_ctx_t		h_ctx
		);

/** [WIP] */
aclty_t* aclty_build_arr(
		aclty_t* ae2f_restrict	rdwr_arr,
		const h_aclspv_ctx_t		h_ctx
		);

aclty_t* aclty_cfg_arr(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		);

#endif
