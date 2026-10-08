#ifndef	aclconst_specop_h
#define aclconst_specop_h

#include <aclspv.h>


/**
 * @param rd_wrds	full declaration of OpSpecConstantOp
 * @param c_num_wrd	number of words(aclspv_wrd_t) of rd_wrds
 * */
aclspv_id_t	aclconst_specop(
		aclspv_wrd_t* ae2f_restrict		rd_wrds,
		aclspv_wrdcount_t			c_num_wrd,
		h_aclspv_ctx_t				h_ctx
		);

#endif
