#ifndef	aclutil_cxtp_vec_h
#define	aclutil_cxtp_vec_h

#include "./common.h"
#include <ae2f/cc/fnpretty.h>

ae2f_ccpure struct aclutil_cxtp_veccfg_t {
	aclspv_id_t	m_ty_elm;
	aclspv_wrd_t	m_num;
}	 aclty_get_veccfg(
		const aclspv_id_t	c_ty_vec,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclty_mk_vec(
		const aclspv_id_t	c_type_elm,
		const aclspv_wrd_t	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		);


#endif
