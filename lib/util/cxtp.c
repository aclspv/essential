
#include "./cxtp.h"

#include "./constant.h"
#include "./log.h"
#include "./iddef.h"

#include <ae2f/bll.h>
#include <ae2f/pat/dbg.h>

#if	1
#include <aclattr/storage_class.h>
#include <aclattr/specid.h>
#endif

ae2f_ccconst aclspv_id_t aclutil_mk_cxtp_by_type(const CXType c_ty) {
	switch((uint_leastenum_t)c_ty.kind) {
#define	VECCASE(c_case, c_ret)	\
		case c_case: return c_ret

		case CXType_SChar:
		VECCASE(CXType_Char_S		, ID_DEFAULT_I8);

		case CXType_UChar:
		VECCASE(CXType_Char_U		, ID_DEFAULT_U8);

		case CXType_Char16:
		VECCASE(CXType_Short		, ID_DEFAULT_I16);
		VECCASE(CXType_UShort		, ID_DEFAULT_U16);

		case CXType_Char32:
		VECCASE(CXType_Int		, ID_DEFAULT_I32);
		VECCASE(CXType_UInt		, ID_DEFAULT_U32);

		case CXType_LongLong:
		VECCASE(CXType_Long		, ID_DEFAULT_I64);

		case CXType_ULongLong:
		VECCASE(CXType_ULong		, ID_DEFAULT_U64);

		case CXType_BFloat16:
		case CXType_Float16:
		VECCASE(CXType_Half		, ID_DEFAULT_F16);
		VECCASE(CXType_Float		, ID_DEFAULT_F32);
		VECCASE(CXType_Double		, ID_DEFAULT_F64);

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

aclspv_id_t aclutil_mk_cxtp_by_cursor2(
		const CXCursor		c_cursor,
		const h_aclspv_ctx_t	h_ctx
		) {
	const CXCursor	TYPEDEF	= clang_getTypeDeclaration(clang_getCursorType(c_cursor));
	const CXType	TYPE = clang_getCanonicalType((clang_getCursorType(c_cursor)));

	{
		const aclspv_id_t D = aclutil_mk_cxtp_by_type2(TYPE, h_ctx);
		if(D) {
			return D;
		}
	}

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "cxtp_by_cursor2 tykind %u\n", (unsigned)TYPE.kind));

	switch((uint_leastenum_t)TYPE.kind) {
		case CXType_FunctionProto:
			{
				const CXType CANONICAL	= clang_getCanonicalType(TYPE);
				return aclutil_mk_cxtp_fn1(
						c_cursor
						, CANONICAL
						, h_ctx);
			}
		case CXType_FunctionNoProto:
			{
				const CXType RET	= clang_getResultType(TYPE);
				const aclspv_id_t RETID	= aclutil_mk_cxtp_by_cursor(c_cursor, RET, h_ctx);

				return aclutil_mk_cxtp_fn(RETID, h_ctx);
			}

		case CXType_Pointer:
			{
				const CXType ELM	= clang_getPointeeType(TYPE);
				const aclspv_id_t ELMID	= aclutil_mk_cxtp_by_cursor(c_cursor, ELM, h_ctx);
				enum SpvStorageClass_	SCLASS = SpvStorageClassMax;

				clang_visitChildren(TYPEDEF, s_attr_storage_class, &SCLASS);
				clang_visitChildren(c_cursor, s_attr_storage_class, &SCLASS);

				SCLASS = (enum SpvStorageClass_)ae2f_bll_sel_bool1(
						uint_leastenum_t
						, SCLASS == SpvStorageClassMax
						, SpvStorageClassFunction	/* default fallback */
						, (uint_least32_t) SCLASS
						);

				return aclutil_mk_cxtp_ptr(ELMID, SCLASS, h_ctx);
			}

		case CXType_ConstantArray:	/** array or */
			{
				enum ACLUTIL_CXTYPE_ E		= ACLUTIL_CXTYPE_ARRAY;
				const CXType ELM		= clang_getArrayElementType(TYPE);
				const intmax_t _ELMCOUNT	= clang_getNumElements(TYPE);
				const aclspv_wrd_t ELMCOUNT	= 
					ae2f_bll_sel_bool_naive1(
							aclspv_wrd_t
							, (umax)(_ELMCOUNT) & ~(umax)ACLSPV_MASK_WRD
							, 0 
							,(aclspv_wrd_t)_ELMCOUNT
							);

				const aclspv_id_t	ELMID = aclutil_mk_cxtp_by_cursor(c_cursor, ELM, h_ctx);

				aclspv_wrd_t	SPECID = 0xFFFFFFFF;

				if(E == ACLUTIL_CXTYPE_VECTOR) {
					return aclutil_mk_cxtp_vec(
							ELMID
							, ELMCOUNT
							, h_ctx);
				}

				clang_visitChildren(TYPEDEF, s_attr_specid, &SPECID);
				clang_visitChildren(c_cursor, s_attr_specid, &SPECID);

				if(SPECID ^ 0xFFFFFFFF) {
					/** specid */
					aclspv_id_t SPECID_WRD = aclutil_mk_constant_spec_id(
							SPECID, ELMCOUNT, h_ctx);
					unless(SPECID_WRD)
						SPECID_WRD = aclutil_mk_constant_spec64_id(
								SPECID, ELMCOUNT, h_ctx);

					ae2f_expected_but_else(SPECID_WRD) {
						h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
						return 0; /** not found */
					}

					return aclutil_mk_cxtp_arr(
							ELMID, SPECID_WRD
							, h_ctx);
				}

				return aclutil_mk_cxtp_arr(ELMID, ELMCOUNT, h_ctx);
			}

		default:
			return 0;
	}

	ae2f_unreachable();
}

aclspv_id_t aclutil_mk_cxtp_by_cursor(
		const CXCursor		c_cursor,
		const CXType		c_ty,
		const h_aclspv_ctx_t	h_ctx
		)
{
	const aclspv_id_t R0	= aclutil_mk_cxtp_by_cursor2(c_cursor, h_ctx);
	const aclspv_id_t R1	= R0 ? R0 : aclutil_mk_cxtp_by_cursor2(clang_getCanonicalCursor(c_cursor), h_ctx);
	const aclspv_id_t R	= R1 ? R1 : aclutil_mk_cxtp_by_cursor2(clang_getTypeDeclaration(c_ty), h_ctx);
	return R;
}

ae2f_ccpure	aclutil_cxtp* aclutil_find_cxtp(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx) {
	ae2fpat_assertassume_strict(h_ctx);
	{
		size_t	C = (size_t)h_ctx->m_num_type_uniques;
		aclutil_cxtp* ae2f_restrict P =  (aclutil_cxtp* ae2f_restrict)h_ctx->m_type_uniques.m_p;
		ae2fpat_assertassume_strict(C <= h_ctx->m_type_uniques.m_sz / sizeof(aclutil_cxtp));
		while( C --> 0 && (P++)->m_type_id != c) {}
		return ae2f_expected_strict(
				(C / sizeof(aclutil_cxtp)) < (size_t)(h_ctx->m_num_type_uniques)
				) ? P - 1 : 0;
	}
}
