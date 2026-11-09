#ifndef	aclexpr_visit_op1_glslmath_h
#define	aclexpr_visit_op1_glslmath_h

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclspv.h>

/** handles the GLSL.std.450 unary float math builtins, all of which take
 * exactly one argument:
 *
 *   __builtin_sinf / __builtin_cosf / __builtin_tanf      (f32)
 *   __builtin_sinf16 / __builtin_cosf16 / __builtin_tanf16 (f16)
 *   __builtin_elementwise_sin / __builtin_elementwise_cos /
 *   __builtin_elementwise_tan                             (f32 scalar/vector)
 *
 * the spellings share one visitor: the builtin's own signature fixes the
 * expected operand type (f32 for `*f`, f16 for `*f16`), and the generic
 * elementwise spelling expects a simd float. the expected type rides the
 * scale's RESTYID slot; the carve does the conversion.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op1_glsl_math(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		);

#endif
