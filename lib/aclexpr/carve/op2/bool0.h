#ifndef	aclexpr_carve_op2_bitwise_h
#define	aclexpr_carve_op2_bitwise_h

#include <aclexpr/carve/api.h>

aclspv_bool_t aclexpr_carve2_bool0(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		h_scale_t			h_lst_scale,
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		);

#endif
