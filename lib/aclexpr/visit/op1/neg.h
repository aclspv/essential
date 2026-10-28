#ifndef	aclexpr_visit_op1_neg_h
#define aclexpr_visit_op1_neg_h

#include <aclspv.h>
#include <aclexpr.h>
#include <aclexpr/visit/api.h>

aclexpr_visit_lbl_t aclexpr_visit_op1_neg(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		);


#endif
