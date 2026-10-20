/** 
 * @file	simple1.h
 * @brief	int & float literals, variable references
 * */

#ifndef aclexpr_visit_val0_h
#define aclexpr_visit_val0_h

#include <util/emitx.h>
#include <util/constant.h>

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <aclcur/common.h>
#include <aclcur/load/get.h>
#include <aclexpr/curreg.h>

#include <.aclspv/util/mkname.h>
#include <aclexpr/visit/api.h>

/** 
 * binary expression with 1 operators.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op0_val(
		const CXCursor			c_cur,
		const aclutil_fnctx* ae2f_restrict	h_fnctx,
		aclexpr_opbincfg_extra*		rdwr_extra,
		aclspv_id_t* ae2f_restrict const	wr_newid,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		);
#endif
