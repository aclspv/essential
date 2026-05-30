#ifndef	aclutil_vec_h
#define aclutil_vec_h

#include "./literal.h"

/** @brief error could be emitted */
aclspv_id_t aclutil_mkvec(
		aclspv_wrd_t				c_num_elm,
		aclspv_id_t				c_ty,
		const aclspv_id_t* ae2f_restrict	rd_elm,
		aclutil_ltrlmask_t			c_literal,
		h_aclspv_ctx_t				h_ctx
		);

#endif
