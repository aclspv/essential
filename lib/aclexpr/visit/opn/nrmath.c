#include "./nrmath.h"

#if	__ACLSPV_MATH_NR

#include "./nrmath/api.h"
#include "./nrmath/op1/rsqrt.h"

#include <string.h>
#include <util/scale.h>
#include <util/wrdemit.h>
#include <aclexpr/op.h>

aclexpr_visit_lbl_t aclexpr_visit_opn_math_nr(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	typedef aclspv_wrd_t cxkind_t;
	ae2f_unused typedef aclspv_wrd_t cxkind_size_t[sizeof(cxkind_t) >= sizeof(c_cur.kind) ? 1 : -1];

	CXString spl;

	if(c_cur.kind != CXCursor_CallExpr)
		return c_lbl_pass;

#define	retspl(a)	{ clang_disposeString(spl); return (a); }

	spl = clang_getCursorSpelling(c_cur);

	unless(strcmp("__builtin_rsqrtf_nr", spl.data)) 
		switch(aclexpr_visit_op1_rsqrtf_nr(
					c_cur, h_ctx, h_cmdscale
					)) {
			case S_RET_FAIL: retspl(c_lbl_fail);
			case S_RET_GOOD: retspl(c_lbl_success); case S_RET_SKIP: break;
			default: ae2f_unreachable();
		}

	retspl(c_lbl_pass);
}

#endif
