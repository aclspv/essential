#ifndef	aclexpr_carve_op1_glslmath_h
#define	aclexpr_carve_op1_glslmath_h

#include <aclexpr.h>
#include <aclexpr/op.h>
#include <aclexpr/carve/api.h>

/** one carve per GLSL.std.450 unary math instruction.
 *
 * RESTYID carries the builtin's expected operand type, or 0 for a generic
 * spelling. a fixed type is cast from the operand; a generic one uses the
 * operand when it is already a simd float, otherwise the configured default
 * scalar with the same shape (`i16x4 -> f32x4`).
 * */

aclspv_bool_t aclexpr_carve1_sin(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		);

aclspv_bool_t aclexpr_carve1_cos(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		);

aclspv_bool_t aclexpr_carve1_tan(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		);

#endif
