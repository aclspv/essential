#include "./by_type.h"
#include "./vec.h"

#include <util/iddef.h>
#include <util/log.h>

#include <ae2f/bll.h>

ae2f_ccconst aclspv_id_t aclutil_mk_cxtp_by_type(const CXType c_ty) {
	switch((uint_leastenum_t)c_ty.kind) {
#define	VECCASE(c_case, c_ret)	\
		case c_case: return c_ret

		case CXType_SChar:
		VECCASE(CXType_Char_S		, ACLSPV_ID_I8);
		VECCASE(CXType_Bool		, ACLSPV_ID_BOOL);

		case CXType_UChar:
		VECCASE(CXType_Char_U		, ACLSPV_ID_U8);

		case CXType_Char16:
		VECCASE(CXType_Short		, ACLSPV_ID_I16);
		VECCASE(CXType_UShort		, ACLSPV_ID_U16);

		case CXType_Char32:
		VECCASE(CXType_Int		, ACLSPV_ID_I32);
		VECCASE(CXType_UInt		, ACLSPV_ID_U32);

		case CXType_LongLong:
		VECCASE(CXType_Long		, ACLSPV_ID_I64);

		case CXType_ULongLong:
		VECCASE(CXType_ULong		, ACLSPV_ID_U64);

		case CXType_BFloat16:
		case CXType_Float16:
		VECCASE(CXType_Half		, ACLSPV_ID_F16);
		VECCASE(CXType_Float		, ACLSPV_ID_F32);
		VECCASE(CXType_Double		, ACLSPV_ID_F64);

#undef		VECCASE

		default: return 0;
	}
}

aclspv_id_t aclutil_mk_cxtp_by_type2(
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		) {
	const CXType TYPE = clang_getCanonicalType(c_type);

	{
		const aclspv_id_t D = aclutil_mk_cxtp_by_type(TYPE);
		if(D) {
			return aclid_mk_default_id(D, h_ctx);
		}
	}

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "cxtp_by_type2 tykind %u\n", (unsigned)TYPE.kind));

	switch((uint_leastenum_t)TYPE.kind) {
		case CXType_Vector: case CXType_ExtVector:
			{
				const CXType ELM		= clang_getElementType(TYPE);
				const intmax_t _ELMCOUNT	= clang_getNumElements(TYPE);
				const aclspv_wrd_t ELMCOUNT	= 
					ae2f_bll_sel_bool_naive1(
							aclspv_wrd_t
							, (umax)(_ELMCOUNT) & ~(umax)ACLSPV_MASK_WRD
							, 1 
							,(aclspv_wrd_t)_ELMCOUNT
							);

				const aclspv_id_t	ELMID = aclutil_mk_cxtp_by_type(ELM);

				return aclutil_mk_cxtp_vec(
						ELMID
						, ELMCOUNT
						, h_ctx);
			}

		default:
			return 0;
	}

	ae2f_unreachable();
}
