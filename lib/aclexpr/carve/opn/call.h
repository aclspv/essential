#ifndef	aclexpr_carve_opn_call_h
#define	aclexpr_carve_opn_call_h

#include <aclexpr/carve/api.h>

/**
 * build OpFunctionCall: [result type][result][callee][args...].
 *
 * `c_fn` is the callee's OpFunction result id (not a value), `c_args` are
 * the argument values already converted to the callee's parameter types.
 *
 * an impure callee invalidates every tracked register value — the call may
 * have written state this function does not own, so each register's next
 * read must load again. a pure / const callee promises it did not, and the
 * caches stay valid.
 *
 * `wr_is_pure` reports that promise to the caller (it drives the NOPURE
 * propagation at the expression level).
 * */
aclspv_bool_t aclexpr_call_make(
		aclspv_id_t		c_fn,
		aclty_id_t		c_ty_ret,
		const aclspv_vid_t	c_args[],
		aclspv_wrd_t		c_num,
		aclspv_id_t* ae2f_restrict	wr_id,
		aclspv_bool_t* ae2f_restrict	wr_is_pure,
		h_aclspv_ctx_t		h_ctx
		);

aclspv_bool_t aclexpr_carve_opn_call(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		acltrl_t* ae2f_restrict			r_ltrl
		);

#endif
