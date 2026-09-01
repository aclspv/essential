#ifndef	aclutil_cxtp_by_type_h
#define	aclutil_cxtp_by_type_h

#include "./common.h"
#include <ae2f/cc/fnpretty.h>

ae2f_ccconst aclspv_id_t aclty_mk_by_type(const CXType c_ty);
aclspv_id_t aclutil_mk_cxtp_by_type2(
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		);

#endif
