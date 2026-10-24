#include <aclspv.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/visit/opn/nrmath/api.h>

enum S_RET_ aclexpr_visit_op1_rsqrtf_nr(
		const CXCursor c_cur,
		const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const h_cmdscale
		);
