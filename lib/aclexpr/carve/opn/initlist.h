/**
 * @file	initlist.h
 * @brief	aggregate value production (see visit/opn/initlist.h)
 * */

#ifndef	aclexpr_carve_opn_initlist_h
#define	aclexpr_carve_opn_initlist_h

#include <aclexpr/carve/api.h>

aclspv_bool_t aclexpr_carve_opn_initlist(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		aclutil_literal* ae2f_restrict		r_ltrl
		);

#endif

