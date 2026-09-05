#include "./fn.h"
#include "./fn/arg.h"
#include "./fn/build.h"

#include "./implutil.h"
#include "./by_cursor.h"


#if	0
static void s_cur_dcr(aclutil_cxtp* cxtp, CXCursor c_cur, CXType c_ty, h_aclspv_ctx_t h_ctx) {

}
#endif

aclspv_id_t aclty_mk_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclty_t* cxtp = aclty_cfg_fn(c_ret_id, h_ctx);
	ae2f_ifezerr(cxtp)	return 0;
	return aclty_build_fn(cxtp, h_ctx);
}

aclspv_id_t aclty_mk_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		)
{
	int ARGC			= clang_getNumArgTypes(c_type_compound);
	aclspv_wrd_t	ARGC_IDX	= (aclspv_wrd_t)ARGC;
	aclspv_id_t	RETID;
	aclty_t* fn;
	aclspv_wrd_t	pad;

	{
		const CXType RET	= clang_getResultType(c_type_compound);
		RETID			= aclty_mk_by_cursor(
				c_cursor_compound
				,RET 
				, h_ctx);

		ae2f_expected_but_else(RETID) return 0;

	}


	ae2f_ifnezerr_strict(ARGC <= 0) {
		return aclty_mk_fn(RETID, h_ctx);
	}	/** fallback: argc was under 0 */

	ae2f_ifezerr(fn = aclty_cfg_fn(RETID, h_ctx))
		return 0;


	pad = (aclspv_wrd_t)(fn - __get_type_uniques(h_ctx));

	while(ARGC_IDX--) {
		const CXType ARG		= clang_getArgType(
				c_type_compound
				, ARGC_IDX
				);

		const aclspv_wrd_t ARG_ID	= aclty_mk_by_cursor(
				c_cursor_compound
				, ARG
				, h_ctx
				);

		ae2f_expected_but_else(ARG_ID)	{
			h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
			return 0;
		}

		ae2f_ifezerr(aclty_add_fn_arg(ARG_ID, h_ctx, fn))
			return 0;

		fn = &__get_type_uniques(h_ctx)[pad];
	}

	return aclty_build_fn(fn, h_ctx);
}
