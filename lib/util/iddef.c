/** @file iddef.c */

#include "./iddef.h"
#include "./emitx.h"
#include "./cxtp.h"
#include "./capability.h"

#include <assert.h>
#include <spirv/unified1/spirv.h>

ae2f_ccconst aclspv_bool_t aclid_is_castable(const B_id_default_t c_id, const h_aclspv_ctx_t h_ctx) {
	return aclid_is_arithmetic(c_id, h_ctx);
}

ae2f_ccconst aclspv_bool_t aclid_is_castable1(const B_id_default_t c_id0, const B_id_default_t c_id1, const h_aclspv_ctx_t h_ctx) {
	return aclid_is_arithmetic(c_id0, h_ctx) && aclid_is_arithmetic(c_id1, h_ctx);
}

ae2f_ccconst aclspv_bool_t aclid_is_unsigned(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ID_DEFAULT_U8:
		case ID_DEFAULT_U16:
		case ID_DEFAULT_U32:
		case ID_DEFAULT_U64:
			return 1;

		case ID_DEFAULT_I32:
		case ID_DEFAULT_I8:
		case ID_DEFAULT_I16:
		case ID_DEFAULT_I64:
		case ID_DEFAULT_F16:
		case ID_DEFAULT_F32:
		case ID_DEFAULT_F64:
		case ID_DEFAULT_END:
		case ID_DEFAULT_VOID:
		default:
			return 0;
	}

	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t aclid_is_signed(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ID_DEFAULT_I32:
		case ID_DEFAULT_I8:
		case ID_DEFAULT_I16:
		case ID_DEFAULT_I64:
			return 1;

		case ID_DEFAULT_F16:
		case ID_DEFAULT_F32:
		case ID_DEFAULT_F64:
		case ID_DEFAULT_U8:
		case ID_DEFAULT_U16:
		case ID_DEFAULT_U32:
		case ID_DEFAULT_U64:
		case ID_DEFAULT_END:
		case ID_DEFAULT_VOID:
		default:
			return 0;
	}

	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t aclid_is_int(const B_id_default_t c_id_default) {
	return aclid_is_unsigned(c_id_default) | aclid_is_signed(c_id_default);
}

ae2f_ccconst aclspv_bool_t aclid_is_float(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ID_DEFAULT_F16:
		case ID_DEFAULT_F32:
		case ID_DEFAULT_F64:
			return 1;

		case ID_DEFAULT_I8:
		case ID_DEFAULT_I16:
		case ID_DEFAULT_I32:
		case ID_DEFAULT_I64:
		case ID_DEFAULT_U8:
		case ID_DEFAULT_U16:
		case ID_DEFAULT_U32:
		case ID_DEFAULT_U64:
		case ID_DEFAULT_END:
		case ID_DEFAULT_VOID:
		default:
			return 0;
	}


	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t aclid_is_default(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ID_DEFAULT_F16:
		case ID_DEFAULT_F32:
		case ID_DEFAULT_F64:
		case ID_DEFAULT_I8:
		case ID_DEFAULT_I16:
		case ID_DEFAULT_I32:
		case ID_DEFAULT_I64:
		case ID_DEFAULT_END:
		case ID_DEFAULT_VOID:
		case ID_DEFAULT_U8:
		case ID_DEFAULT_U16:
		case ID_DEFAULT_U32:
		case ID_DEFAULT_U64:
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
			return ID_DEFAULT_F16;
		case 32:
			return ID_DEFAULT_F32;
		case 64:
			return ID_DEFAULT_F64;
		default:
			return ID_DEFAULT_END;
	}

	ae2f_unreachable();
}

ae2f_ccconst B_id_default_t aclid_getty_signed(unsigned bit_width) {
	switch(bit_width) {
		case 8:
			return ID_DEFAULT_I8;
		case 16:
			return ID_DEFAULT_I16;
		case 32:
			return ID_DEFAULT_I32;
		case 64:
			return ID_DEFAULT_I64;
		default:
			return ID_DEFAULT_END;
	}

	ae2f_unreachable();
}

ae2f_ccconst B_id_default_t aclid_getty_unsigned(unsigned bit_width) {
	switch(bit_width) {
		case 8:
			return ID_DEFAULT_U8;
		case 16:
			return ID_DEFAULT_U16;
		case 32:
			return ID_DEFAULT_U32;
		case 64:
			return ID_DEFAULT_U64;
		default:
			return ID_DEFAULT_END;
	}

	ae2f_unreachable();
}

ae2f_ccconst unsigned aclid_get_bit_width(const B_id_default_t c_id_default) {
	switch(c_id_default) {
		case ID_DEFAULT_BOOL:
			return 1;

		case ID_DEFAULT_I8:
		case ID_DEFAULT_U8:
			return 8;

		case ID_DEFAULT_I16:
		case ID_DEFAULT_U16:
		case ID_DEFAULT_F16:
			return 16;

		case ID_DEFAULT_I32:
		case ID_DEFAULT_U32:
		case ID_DEFAULT_F32:
			return 32;

		case ID_DEFAULT_F64:
		case ID_DEFAULT_I64:
		case ID_DEFAULT_U64:
			return 64;

		case ID_DEFAULT_END:
		case ID_DEFAULT_VOID:
		default:
			return 0;
	}

	ae2f_unreachable();
}

ae2f_ccconst aclspv_bool_t	aclid_is_vectorisable(const aclspv_id_t c_id) {
	return c_id == ID_DEFAULT_BOOL || aclid_is_number(c_id);
}

aclspv_bool_t	aclid_is_arithmetic(const aclspv_id_t c_id, const h_aclspv_ctx_t h_ctx) 
{
	ae2fpat_assertassume(h_ctx);

	if(aclutil_cxtp_get_veccfg(c_id, h_ctx).m_num > 1)
		return 1;

	return aclid_is_vectorisable(c_id);
}

#include <ae2f/bll.h>

/**
 * @fn		util_default_decide_cast
 * @returns	0 when invalid
 * */
ae2f_ccconst B_id_default_t aclid_decide_cast(const e_id_default c_id0, const e_id_default c_id1, const h_aclspv_ctx_t h_ctx) {
	/** one is 0? */
	unless(c_id0 & c_id1)
		return ae2f_bll_sel_bool_naive1(B_id_default_t, c_id0, (B_id_default_t) c_id0, (B_id_default_t) c_id1);

	unless(c_id0 ^ c_id1)
		return (B_id_default_t) c_id0;

	unless(aclid_is_arithmetic((B_id_default_t) c_id0, h_ctx) & aclid_is_arithmetic((B_id_default_t) c_id1, h_ctx))
		return 0;

	if(aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num ^ aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num) {
		unless(aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id1;
		} else unless(aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id0;
		} return 0;
	}

	unless(aclid_is_vectorisable((B_id_default_t) c_id0)) {
		return ae2f_bll_sel_bool1(B_id_default_t, aclid_decide_cast(
				(e_id_default) aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_ty_elm
					, (e_id_default) aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_ty_elm
					, h_ctx
					), (B_id_default_t) c_id0, (B_id_default_t) c_id1);
	}

	unless(aclid_is_number((B_id_default_t) c_id0) & aclid_is_number((B_id_default_t) c_id1))
		return 0;

	if(aclid_is_float((B_id_default_t) c_id0) != aclid_is_float((B_id_default_t) c_id1))
		return ae2f_bll_sel_bool1(B_id_default_t, aclid_is_float((B_id_default_t) c_id0), (B_id_default_t) c_id0, (B_id_default_t)c_id1);

	return ae2f_bll_sel_bool1(
			B_id_default_t
			, aclid_get_bit_width((B_id_default_t) c_id0) > aclid_get_bit_width((B_id_default_t) c_id1)
			, (B_id_default_t) c_id0 , (B_id_default_t) c_id1);
}

aclspv_id_t aclid_mk_default_id(
		const B_id_default_t	c_id_default,
		h_util_ctx_t	h_ctx
		)
{
	ae2fpat_assertassume(h_ctx);
	if(c_id_default == ID_DEFAULT_END)
		return 0;

	if(h_ctx->m_id_defaults[c_id_default])
		return c_id_default;

#define CTX		h_ctx
#define	ret_count	h_ctx->m_count.m_types
	switch(c_id_default) {
		case ID_DEFAULT_END:
		default:
			return 0;

		case ID_DEFAULT_BOOL:
			ae2f_expected_but_else((ret_count = util_emitx_2(
							&CTX->m_section.m_types
							, ret_count
							, SpvOpTypeBool
							, ID_DEFAULT_BOOL)))
				return 0;
			break;

		case ID_DEFAULT_VOID:
			/** OpTypeVoid */
			ae2f_expected_but_else((ret_count = emit_opcode(
							&CTX->m_section.m_types
							, ret_count
							, SpvOpTypeVoid, 1)))
				return 0;
			ae2f_expected_but_else((ret_count = util_emit_wrd(
							&CTX->m_section.m_types
							, ret_count
							, ID_DEFAULT_VOID
							))) 
				return 0;
			break;

		case ID_DEFAULT_I64:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_I64
						, 64, 1))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt64)) 
				return 0;
			break;

		case ID_DEFAULT_U64:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_U64
						, 64, 0))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt64)) 
				return 0;
			break;
		case ID_DEFAULT_U32:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_U32
						, 32, 0))
				return 0;
			break;

		case ID_DEFAULT_I32:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_I32
						, 32, 1))
				return 0;
			break;

		case ID_DEFAULT_U16:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_U16
						, 16, 0))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt16)) 
				return 0;

			break;

		case ID_DEFAULT_I16:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_I16
						, 16, 1))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt16)) 
				return 0;

			break;

		case ID_DEFAULT_U8:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_U8
						, 8, 0))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt8)) 
				return 0;
			break;

		case ID_DEFAULT_I8:
			ae2f_expected_but_else(
					ret_count = util_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ID_DEFAULT_I8
						, 8, 1))
				return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt8)) 
				return 0;
			break;

		case ID_DEFAULT_F16:
			ae2f_expected_but_else(ret_count = util_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ID_DEFAULT_F16
						, 16
						)) return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityFloat16)) 
				return 0;
			break;

		case ID_DEFAULT_F32:
			ae2f_expected_but_else(ret_count = util_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ID_DEFAULT_F32
						, 32
						)) return 0;
			break;

		case ID_DEFAULT_F64:
			ae2f_expected_but_else(ret_count = util_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ID_DEFAULT_F64
						, 64
						)) return 0;

			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityFloat64)) 
				return 0;
			break;
	}

	return h_ctx->m_id_defaults[c_id_default] = c_id_default;
}


ae2f_ccconst B_id_default_t aclid_number_cvtbitcount(const B_id_default_t c_id, const unsigned bitcount) {
	if(aclid_is_unsigned(c_id))
		return aclid_getty_unsigned(bitcount);

	if(aclid_is_signed(c_id))
		return aclid_getty_signed(bitcount);

	if(aclid_is_float(c_id))
		return aclid_getty_float(bitcount);

	return 0;
}
