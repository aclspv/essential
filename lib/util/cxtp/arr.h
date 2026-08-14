#ifndef	aclutil_cxtp_arr_h
#define	aclutil_cxtp_arr_h

#include "./common.h"
#include <.aclspv/const/spec.h>

aclspv_id_t aclutil_mk_cxtp_arr3(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		aclspv_wrdcount_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		);

/** array with stride */
aclspv_id_t aclutil_mk_cxtp_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrdcount_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		);

#define	aclutil_mk_cxtp_arr(c_type_elm, c_num_arr, h_ctx)	\
	aclutil_mk_cxtp_arr2(c_type_elm, c_num_arr, 0xFFFFFFFF, h_ctx)


aclutil_cxtp* aclutil_cfg_cxtp_arr_dcr(
		aclutil_cxtp* ae2f_restrict	rdwr_arr,
		const enum SpvDecoration_	c_dcr,
		const h_aclspv_ctx_t		h_ctx
		);

/** [WIP] */
aclutil_cxtp* aclutil_build_cxtp_arr(
		aclutil_cxtp* ae2f_restrict	rdwr_arr,
		const h_aclspv_ctx_t		h_ctx
		);

aclutil_cxtp* aclutil_cfg_cxtp_arr(
		const aclspv_id_t	c_type_elm,
		const aclconst_specsz	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		);

#endif
