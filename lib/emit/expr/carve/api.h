#ifndef	aclexpr_carve_api_h
#define	aclexpr_carve_api_h

#include <emit/expr/carve.h>
#include <util/scale.h>
#include <ae2f/cgoto.h>


#define	aclspv_cgoto		ae2f_cgoto_available

#if	aclspv_cgoto
typedef	void* 			aclexpr_carve_lbl_t;
#else
#include <ae2f/c90/StdInt/enum.h>
typedef uint_leastenum_t	aclexpr_carve_lbl_t;
#endif

typedef aclexpr_carve_lbl_t aclexpr_carve_api_t(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		h_scale_t					h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		util_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		util_literal* ae2f_restrict			r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		);

#endif
