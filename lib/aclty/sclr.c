#include "./sclr.h"

#include <aclutil/emitx.h>
#include <aclutil/capability.h>
#include <ae2f/pat/dbg.h>

ae2f_ccconst aclspv_bool_t aclty_is_sclr_uint(const aclty_sclr_t c) {
	switch(c) {
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

ae2f_ccconst aclspv_bool_t aclty_is_sclr_sint(const aclty_sclr_t c) {
	switch(c) {
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


ae2f_ccconst aclspv_bool_t aclty_is_sclr_signed(const aclty_sclr_t c) {
	switch(c) {
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

ae2f_ccconst aclspv_bool_t aclty_is_sclr_int(const aclty_sclr_t c_id_default) {
	return aclty_is_sclr_uint(c_id_default) || aclty_is_sclr_sint(c_id_default);
}

ae2f_ccconst aclspv_bool_t aclty_is_sclr_flt(const aclty_sclr_t c_id_default) {
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

ae2f_ccconst aclspv_bool_t aclty_is_default(const aclty_default_t c_id_default) {
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
aclty_is_sclr_num(const aclty_sclr_t c_id_default) {
	return aclty_is_sclr_int(c_id_default) || aclty_is_sclr_flt(c_id_default);
}

ae2f_ccconst aclty_default_t aclty_get_sclr_flt(aclty_sclr_width_t bit_width) {
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

ae2f_ccconst aclty_default_t aclty_get_sclr_sint(aclty_sclr_width_t bit_width) {
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

ae2f_ccconst aclty_default_t aclty_get_sclr_uint(aclty_sclr_width_t bit_width) {
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

ae2f_ccconst aclspv_uiter_t aclty_get_sclr_width(const aclty_sclr_width_t c_id_default) {
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

aclspv_id_t aclty_mk_default(
		const aclty_default_t	c_id_default,
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
			ae2f_ifezerr((ret_count = aclutil_emitx_2(
							&CTX->m_section.m_types
							, ret_count
							, SpvOpTypeBool
							, ACLSPV_ID_BOOL)))
				return 0;
			break;

		case ACLSPV_ID_VOID:
			/** OpTypeVoid */
			ae2f_ifezerr((ret_count = aclutil_emitx_2(
							&CTX->m_section.m_types
							, ret_count
							, SpvOpTypeVoid
							, ACLSPV_ID_VOID)))
				return 0;
			break;

		case ACLSPV_ID_I64:
			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt64)) 
				return 0;

			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I64
						, 64, 1))
				return 0;

			break;

		case ACLSPV_ID_U64:
			ae2f_expected_but_else(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt64))
				return 0;

			ae2f_expected_but_else(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U64
						, 64, 0))
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

			ae2f_ifezerr(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U16
						, 16, 0))
				return 0;

			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt16)) 
				return 0;

			break;

		case ACLSPV_ID_I16:
			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt16)) 
				return 0;

			ae2f_ifezerr(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I16
						, 16, 1))
				return 0;

			break;

		case ACLSPV_ID_U8:
			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt8)) 
				return 0;

			ae2f_ifezerr(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_U8
						, 8, 0))
				return 0;
			break;

		case ACLSPV_ID_I8:
			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityInt8)) 
				return 0;

			ae2f_ifezerr(
					ret_count = aclutil_emitx_4(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeInt
						, ACLSPV_ID_I8
						, 8, 1))
				return 0;

			break;

		case ACLSPV_ID_F16:
			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityFloat16))
				return 0;

			ae2f_ifezerr(ret_count = aclutil_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ACLSPV_ID_F16
						, 16
						)) return 0;
			break;

		case ACLSPV_ID_F32:
			ae2f_ifezerr(ret_count = aclutil_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ACLSPV_ID_F32
						, 32
						)) return 0;
			break;

		case ACLSPV_ID_F64:
			ae2f_ifezerr(aclutil_add_capability_by_flag(CTX, SpvCapabilityFloat64)) 
				return 0;

			ae2f_ifezerr(ret_count = aclutil_emitx_3(
						&CTX->m_section.m_types
						, ret_count
						, SpvOpTypeFloat
						, ACLSPV_ID_F64
						, 64
						)) return 0;

			break;
	}

	return h_ctx->m_id_defaults[c_id_default] = c_id_default;
}


ae2f_ccconst aclty_default_t aclty_cvt_sclr_width(const aclty_sclr_t c_id, const aclty_sclr_width_t bitcount) {
	if(aclty_is_sclr_uint(c_id))
		return aclty_get_sclr_uint(bitcount);

	if(aclty_is_sclr_sint(c_id))
		return aclty_get_sclr_sint(bitcount);

	if(aclty_is_sclr_flt(c_id))
		return aclty_get_sclr_flt(bitcount);

	return (aclty_default_t)ae2f_expected_not(0);
}
