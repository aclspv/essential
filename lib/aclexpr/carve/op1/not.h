#ifndef	aclexpr_carve_op1_not_h
#define	aclexpr_carve_op1_not_h

#include <aclexpr.h>
#include <aclexpr/op.h>
#include <aclexpr/carve/api.h>

/** `!x`: OpLogicalNot over a bool-cast operand. */
aclspv_bool_t aclexpr_carve1_not(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		);

/** `~x`: OpNot on the promoted integer result type. */
aclspv_bool_t aclexpr_carve1_bitnot(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		);

#endif
