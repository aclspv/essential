#ifndef lib_util_emitx
#define lib_util_emitx

#include <aclutil/wrdemit.h>

ae2f_inline static aclspv_sz_t	aclutil_emitx_2(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t				c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_oprand
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 1u, c_oprand))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 1u);

	return c_wrdcount + 2u;
}

ae2f_inline static aclspv_sz_t	aclutil_emitx_3(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t				c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_opr_0,
		const aclspv_wrd_t			c_opr_1
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 2u, c_opr_1))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 1u]	= c_opr_0;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 2u);

	return c_wrdcount + 3u;
}

ae2f_inline static aclspv_sz_t	aclutil_emitx_4(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t				c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_opr_0,
		const aclspv_wrd_t			c_opr_1,
		const aclspv_wrd_t			c_opr_2
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 3u, c_opr_2))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 2u] 	= c_opr_1;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 1u]	= c_opr_0;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 3u); 

	return c_wrdcount + 4u;
}

ae2f_inline static aclspv_sz_t	aclutil_emitx_5(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t				c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_opr_0,
		const aclspv_wrd_t			c_opr_1,
		const aclspv_wrd_t			c_opr_2,
		const aclspv_wrd_t			c_opr_3
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 4u, c_opr_3))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 3u]	= c_opr_2;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 2u]	= c_opr_1;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 1u]	= c_opr_0;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 4u);

	return c_wrdcount + 5u;
}

ae2f_inline static aclspv_sz_t	aclutil_emitx_6(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t				c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_opr_0,
		const aclspv_wrd_t			c_opr_1,
		const aclspv_wrd_t			c_opr_2,
		const aclspv_wrd_t			c_opr_3,
		const aclspv_wrd_t			c_opr_4
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 5u, c_opr_4))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 4u] = c_opr_3;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 3u] = c_opr_2;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 2u] = c_opr_1;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 1u]	= c_opr_0;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 5u);

	return c_wrdcount + 6u;
}

ae2f_inline static aclspv_sz_t	aclutil_emitx_7(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t				c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_opr_0,
		const aclspv_wrd_t			c_opr_1,
		const aclspv_wrd_t			c_opr_2,
		const aclspv_wrd_t			c_opr_3,
		const aclspv_wrd_t			c_opr_4,
		const aclspv_wrd_t			c_opr_5
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 6u, c_opr_5))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 5u] = c_opr_4;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 4u] = c_opr_3;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 3u] = c_opr_2;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 2u] = c_opr_1;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 1u]	= c_opr_0;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 6u);

	return c_wrdcount + 7u;
}

ae2f_inline static aclspv_sz_t	aclutil_emitx_8(
		x_aclspv_vec* ae2f_restrict const	h_wrds,
		const aclspv_sz_t			c_wrdcount,
		const aclspv_wrd_t			c_opcode,
		const aclspv_wrd_t			c_opr_0,
		const aclspv_wrd_t			c_opr_1,
		const aclspv_wrd_t			c_opr_2,
		const aclspv_wrd_t			c_opr_3,
		const aclspv_wrd_t			c_opr_4,
		const aclspv_wrd_t			c_opr_5,
		const aclspv_wrd_t			c_opr_6
		) 
{
	ae2f_expected_but_else(aclutil_emit_wrd(h_wrds, c_wrdcount + 7u, c_opr_6))
		return 0u;

	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 6u]	= c_opr_5;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 5u]	= c_opr_4;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 4u]	= c_opr_3;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 3u]	= c_opr_2;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 2u]	= c_opr_1;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount + 1u]	= c_opr_0;
	aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]	= aclspv_opcode_to_wrd(c_opcode, 7u);

	return c_wrdcount + 8u;
}

#define	util_emitx_variable(h_wrds, c_wrdcount, c_type, c_retid, c_storage_class)	\
	aclutil_emitx_4(h_wrds, c_wrdcount, SpvOpVariable, c_type, c_retid, c_storage_class)

#define	util_emitx_type_pointer(h_wrds, c_wrdcount, c_retid, c_storage_class, c_elm_type_id)	\
	aclutil_emitx_4(h_wrds, c_wrdcount, SpvOpTypePointer, c_retid, c_storage_class, c_elm_type_id)

#define util_emitx_type_array(h_wrds, c_wrdcount, c_retid, c_elm_type_id, c_arrcount_id)	\
	aclutil_emitx_4(h_wrds, c_wrdcount, SpvOpTypeArray, c_retid, c_elm_type_id, c_arrcount_id)

#define util_emitx_spec_constant(h_wrds, c_wrdcount, c_ty, c_retid, c_val)	\
	aclutil_emitx_4(h_wrds, c_wrdcount, SpvOpSpecConstant, c_ty, c_retid, c_val)

#define	util_emitx_spec_constant_op(h_wrds, c_wrdcount, c_ty, c_retid, c_operator, c_operand)	\
	aclutil_emitx_5(h_wrds, c_wrdcount, SpvOpSpecConstantOp, c_ty, c_retid, c_operator, c_operand)

#define	util_emitx_spec_constant_op2(h_wrds, c_wrdcount, c_retid, c_ty, c_operator, c_opr_0, c_opr_1)	\
	aclutil_emitx_6(h_wrds, c_wrdcount, SpvOpSpecConstantOp, c_retid, c_ty, c_operator, c_opr_0, c_opr_1)

#endif
