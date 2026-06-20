/** @file iddef.c */

#include "./iddef.h"
#include "./emitx.h"
#include "./cxtp/vec.h"
#include "./capability.h"

#include <ae2f/pat/dbg.h>

ae2f_ccconst aclspv_bool_t aclid_is_castable(const B_id_default_t c_id, const h_aclspv_ctx_t h_ctx) {
	return aclid_is_arithmetic(c_id, h_ctx);
}

ae2f_ccconst aclspv_bool_t aclid_is_castable1(const B_id_default_t c_id0, const B_id_default_t c_id1, const h_aclspv_ctx_t h_ctx) {
	return aclid_is_arithmetic(c_id0, h_ctx) && aclid_is_arithmetic(c_id1, h_ctx);
}

ae2f_ccconst aclspv_bool_t aclid_is_uint(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
			return 1;

		case ACLSPV_ID_I32:
		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I64:
		case ACLSPV_ID_F16:
		case ACLSPV_ID_F32:
		case ACLSPV_ID_F64:
		case ACLSPV_ID_END:
		case ACLSPV_ID_VOID:
		default:
			return 0;
	}

	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t aclid_is_sint(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ACLSPV_ID_I32:
		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I64:
			return 1;

		case ACLSPV_ID_F16:
		case ACLSPV_ID_F32:
		case ACLSPV_ID_F64:
		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
		case ACLSPV_ID_END:
		case ACLSPV_ID_VOID:
		default:
			return 0;
	}

	ae2f_unreachable();
}


ae2f_ccconst aclspv_bool_t aclid_is_signed(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ACLSPV_ID_I32:
		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I64:
		case ACLSPV_ID_F16:
		case ACLSPV_ID_F32:
		case ACLSPV_ID_F64:
			return 1;
		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
		case ACLSPV_ID_END:
		case ACLSPV_ID_VOID:
		default:
			return 0;
	}
}

ae2f_ccconst aclspv_bool_t aclid_is_int(const B_id_default_t c_id_default) {
	return aclid_is_uint(c_id_default) | aclid_is_sint(c_id_default);
}

ae2f_ccconst aclspv_bool_t aclid_is_float(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ACLSPV_ID_F16:
		case ACLSPV_ID_F32:
		case ACLSPV_ID_F64:
			return 1;

		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I32:
		case ACLSPV_ID_I64:
		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
		case ACLSPV_ID_END:
		case ACLSPV_ID_VOID:
		default:
			return 0;
	}


	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t aclid_is_default(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ACLSPV_ID_F16:
		case ACLSPV_ID_F32:
		case ACLSPV_ID_F64:
		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I32:
		case ACLSPV_ID_I64:
		case ACLSPV_ID_END:
		case ACLSPV_ID_VOID:
		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
			return 1;
		default:
			return 0;
	}

	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t 
aclid_is_number(const B_id_default_t c_id_default) {
	return aclid_is_int(c_id_default) || aclid_is_float(c_id_default);
}

ae2f_ccconst B_id_default_t aclid_getty_float(unsigned bit_width) {
	switch(bit_width) {
		case 16:
			return ACLSPV_ID_F16;
		case 32:
			return ACLSPV_ID_F32;
		case 64:
			return ACLSPV_ID_F64;
		default:
			return ACLSPV_ID_END;
	}

	ae2f_unreachable();
}

ae2f_ccconst B_id_default_t aclid_getty_sint(unsigned bit_width) {
	switch(bit_width) {
		case 8:
			return ACLSPV_ID_I8;
		case 16:
			return ACLSPV_ID_I16;
		case 32:
			return ACLSPV_ID_I32;
		case 64:
			return ACLSPV_ID_I64;
		default:
			return ACLSPV_ID_END;
	}

	ae2f_unreachable();
}

ae2f_ccconst B_id_default_t aclid_getty_uint(unsigned bit_width) {
	switch(bit_width) {
		case 8:
			return ACLSPV_ID_U8;
		case 16:
			return ACLSPV_ID_U16;
		case 32:
			return ACLSPV_ID_U32;
		case 64:
			return ACLSPV_ID_U64;
		default:
			return ACLSPV_ID_END;
	}

	ae2f_unreachable();
}

ae2f_ccconst unsigned aclid_get_bit_width(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ACLSPV_ID_BOOL:
			return 1;

		case ACLSPV_ID_I8:
		case ACLSPV_ID_U8:
			return 8;

		case ACLSPV_ID_I16:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_F16:
			return 16;

		case ACLSPV_ID_I32:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_F32:
			return 32;

		case ACLSPV_ID_F64:
		case ACLSPV_ID_I64:
		case ACLSPV_ID_U64:
			return 64;

		case ACLSPV_ID_END:
		case ACLSPV_ID_VOID:
		default:
			return 0;
	}

	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t	aclid_is_vectorisable(const aclspv_id_t c_id) {
	return c_id == ACLSPV_ID_BOOL || aclid_is_number(c_id);
}

aclspv_bool_t	aclid_is_arithmetic(const aclspv_id_t c_id, const h_aclspv_ctx_t h_ctx) 
{
	ae2fpat_assertassume(h_ctx);

	if(aclutil_cxtp_get_veccfg(c_id, h_ctx).m_num > 1)
		return 1;

	return aclid_is_vectorisable(c_id);
}

#include <ae2f/bll.h>



aclspv_id_t aclid_mk_default_id(
		const B_id_default_t	c_id_default,
		h_aclspv_ctx_t	h_ctx
		)
{
	ae2fpat_assertassume(h_ctx);

	ae2f_ifezerr(c_id_default < (aclspv_wrd_t)(ACLSPV_ID_END))
		return 0;

	if(h_ctx->m_id_defaults[c_id_default])
		return c_id_default;

#define CTX		h_ctx
#define	ret_count	h_ctx->m_count.m_types
	switch(c_id_default) {
		case ACLSPV_ID_END:
		default:
			return 0;

		case ACLSPV_ID_BOOL:
			ae2f_expected_but_else((ret_count = aclutil_emitx_2(
							&CTX->m_section.m_types
							, ret_count
							, SpvOpTypeBool
							, ACLSPV_ID_BOOL)))
				return 0;
			break;

		case ACLSPV_ID_VOID:
			/** OpTypeVoid */
			ae2f_expected_but_else((ret_count = emit_opcode(
							&CTX->m_section.m_types
							, ret_count
							, SpvOpTypeVoid, 1)))
				return 0;
			ae2f_expected_but_else((ret_count = aclutil_emit_wrd(
							&CTX->m_section.m_types
							, ret_count
							, ACLSPV_ID_VOID
							))) 
				return 0;
			break;

		case ACLSPV_ID_I64:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I64
						, 64, 1))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt64)) 
				return 0;
			break;

		case ACLSPV_ID_U64:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U64
						, 64, 0))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt64)) 
				return 0;
			break;
		case ACLSPV_ID_U32:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U32
						, 32, 0))
				return 0;
			break;

		case ACLSPV_ID_I32:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I32
						, 32, 1))
				return 0;
			break;

		case ACLSPV_ID_U16:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U16
						, 16, 0))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt16)) 
				return 0;

			break;

		case ACLSPV_ID_I16:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I16
						, 16, 1))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt16)) 
				return 0;

			break;

		case ACLSPV_ID_U8:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U8
						, 8, 0))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt8)) 
				return 0;
			break;

		case ACLSPV_ID_I8:
			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I8
						, 8, 1))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt8)) 
				return 0;
			break;

		case ACLSPV_ID_F16:
			ae2f_expected_but_else(ret_count = aclutil_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ACLSPV_ID_F16
						, 16
						)) return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityFloat16)) 
				return 0;
			break;

		case ACLSPV_ID_F32:
			ae2f_expected_but_else(ret_count = aclutil_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ACLSPV_ID_F32
						, 32
						)) return 0;
			break;

		case ACLSPV_ID_F64:
			ae2f_expected_but_else(ret_count = aclutil_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ACLSPV_ID_F64
						, 64
						)) return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityFloat64)) 
				return 0;
			break;
	}

	return h_ctx->m_id_defaults[c_id_default] = c_id_default;
}


ae2f_ccconst B_id_default_t aclid_number_cvtbitcount(const B_id_default_t c_id, const unsigned bitcount) {
	if(aclid_is_uint(c_id))
		return aclid_getty_uint(bitcount);

	if(aclid_is_sint(c_id))
		return aclid_getty_sint(bitcount);

	if(aclid_is_float(c_id))
		return aclid_getty_float(bitcount);

	return 0;
}
