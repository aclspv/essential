#include "./by_cursor.h"
#include "./by_type.h"
#include "./struct/mbr.h"
#include "./struct.h"
#include "./fn.h"
#include "./ptr.h"
#include "./arr.h"
#include "../aclty.h"

#include <util/log.h>
#include <util/constant.h>

#include <ae2f/bll.h>

#if	1
#include <aclattr/storage_class.h>
#include <aclattr/specid.h>
#endif

typedef struct {
	aclspv_id_t			m_struct;
	aclty_add_struct_mbr_t*	mfn_add_mbr;
	h_aclspv_ctx_t			m_ctx;
} field_visit_t;

static enum CXChildVisitResult s_field_visit(CXCursor c, CXCursor c_par, CXClientData h);

static aclspv_id_t s_helper2(
		const CXCursor		c_cursor,
		const CXType		c_ty,
		const h_aclspv_ctx_t	h_ctx
		) {
	const CXCursor	TYPEDEF	= clang_getTypeDeclaration(c_ty);
	const CXType	TYPE = clang_getCanonicalType((c_ty));

	{
		const aclspv_id_t D = aclutil_mk_cxtp_by_type2(TYPE, h_ctx);
		if(D) return D;
	}

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "cxtp_by_cursor2 tykind %u\n", (unsigned)TYPE.kind));

	switch((uint_leastenum_t) TYPE.kind) {
		case CXType_Record:
			{
				field_visit_t	VST;
				switch((uint_leastenum_t)clang_getCursorKind(c_cursor)) {
					case CXCursor_UnionDecl:
						VST.mfn_add_mbr = aclty_add_struct_mbr_un;
						break;
					case CXCursor_StructDecl:
						VST.mfn_add_mbr = aclty_add_struct_mbr_std430;
						break;
					default:
						return 0;
				}


				VST.m_ctx = h_ctx;
				VST.m_struct = aclty_cfg_struct(h_ctx);

				clang_visitChildren(
						c_cursor
						, s_field_visit
						, &VST
						);

				{
					const aclty_t* const ae2f_restrict r
					= aclty_build_struct(
							VST.m_struct
							, h_ctx);

					ae2f_ifneznrm(r)
						return r->m_type_id;
				}

				return 0;
			}

		case CXType_FunctionProto:
			{
				const CXType CANONICAL	= TYPE; /** clang_getCanonicalType(TYPE); */
				return aclty_mk_fn1(
						c_cursor
						, CANONICAL
						, h_ctx);
			}

		case CXType_FunctionNoProto:
			{
				const CXType RET	= clang_getResultType(TYPE);
				const aclspv_id_t RETID	= aclty_mk_by_cursor(c_cursor, RET, h_ctx);
				return aclty_mk_fn(RETID, h_ctx);
			}

		case CXType_Pointer:
			{
				const CXType ELM	= clang_getPointeeType(TYPE);
				const aclspv_id_t ELMID	= aclty_mk_by_cursor(c_cursor, ELM, h_ctx);
				enum SpvStorageClass_	SCLASS = SpvStorageClassMax;

				clang_visitChildren(TYPEDEF, s_attr_storage_class, &SCLASS);
				clang_visitChildren(c_cursor, s_attr_storage_class, &SCLASS);

				SCLASS = (enum SpvStorageClass_)ae2f_bll_sel_bool1(
						uint_leastenum_t
						, SCLASS == SpvStorageClassMax
						, SpvStorageClassFunction	/* default fallback */
						, (uint_least32_t) SCLASS
						);

				return aclty_mk_ptr(ELMID, SCLASS, h_ctx);
			}

		case CXType_ConstantArray:	/** array */
			{
				const CXType ELM		= clang_getArrayElementType(TYPE);
				const intmax_t _ELMCOUNT	= clang_getNumElements(TYPE);
				const aclspv_wrd_t ELMCOUNT	= 
					ae2f_bll_sel_bool_naive1(
							aclspv_wrd_t
							, (umax)(_ELMCOUNT) & ~(umax)ACLSPV_MASK_WRD
							, 0 
							,(aclspv_wrd_t)_ELMCOUNT
							);

				const aclspv_id_t	ELMID = aclty_mk_by_cursor(c_cursor, ELM, h_ctx);

				aclspv_wrd_t	SPECID = 0xFFFFFFFF;

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

					return aclty_mk_arr(
							ELMID, SPECID_WRD
							, h_ctx);
				}

				{
					aclconst_specsz C;
					C.m_id = 0;
					C.m_lit.m_max = ELMCOUNT;
					return aclty_mk_arr3(ELMID, C, 0xFFFFFFFF, h_ctx);
				}

#if	0
				return aclutil_mk_cxtp_arr(ELMID, ELMCOUNT, h_ctx);
#endif
			}

		default:
			return 0;
	}

	ae2f_unreachable();
}

static aclspv_id_t s_helper(
		const CXCursor		c_cursor,
		const CXType		c_ty,
		const h_aclspv_ctx_t	h_ctx
		)
{
	{
		const aclspv_id_t R0	= s_helper2(c_cursor, c_ty, h_ctx);
		const aclspv_id_t R1	= R0 ? R0 : s_helper2(clang_getCanonicalCursor(c_cursor), c_ty, h_ctx);
		const aclspv_id_t R	= R1 ? R1 : s_helper2(clang_getTypeDeclaration(c_ty), c_ty, h_ctx);
		return R;
	}
}

static enum CXChildVisitResult s_field_visit(CXCursor c, ae2f_unused CXCursor c_par, CXClientData ae2f_restrict h) {
	field_visit_t* ae2f_restrict h_visit = h;
	aclspv_id_t TYID;
	CXType TY;
	aclty_t* ae2f_restrict FIELD_PTR;

	ae2f_ifnezerr_strict(clang_getCursorKind(c) != CXCursor_FieldDecl) {
		return CXChildVisit_Break;
	}

	TY = clang_getCursorType(c);
	TYID = aclty_mk_by_cursor(c, TY, h_visit->m_ctx);

	FIELD_PTR = aclty_find(h_visit->m_struct, h_visit->m_ctx);

	h_visit->mfn_add_mbr(FIELD_PTR, TYID, h_visit->m_ctx);
	return CXChildVisit_Continue;
}

aclspv_id_t aclty_mk_by_cursor(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		) {
	return s_helper(c_cursor, c_type, h_ctx);
}
