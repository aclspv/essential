
#include "./cxtp.h"
#include "./constant.h"
#include "./emitx.h"
#include "./log.h"
#include "./iddef.h"

#include <ae2f/cc.h>
#include <clang-c/Index.h>
#include <assert.h>

#if	1
#include <attr/storage_class.h>
#include <attr/specid.h>
#endif


ae2f_inline static aclspv_bool_t s_ndeq(const aclutil_cxtp* const ae2f_restrict A, const aclutil_cxtp* const ae2f_restrict B)
{
	if(A->m_class != B->m_class) return 0;
	switch(A->m_class) {
		case ACLUTIL_CXTYPE_FUNC:
			return clang_equalTypes(
					A->m_info.m_fn.m_type
					, B->m_info.m_fn.m_type
					) && clang_equalCursors(
						A->m_info.m_fn.m_cursor
						, B->m_info.m_fn.m_cursor);

		case ACLUTIL_CXTYPE_PTR:
		case ACLUTIL_CXTYPE_VECTOR:
		case ACLUTIL_CXTYPE_ARRAY:
		case ACLUTIL_CXTYPE_FUNC_NOPRM:
		case ACLUTIL_CXTYPE_STRUCT_SINGLE:
		default:
			return !memcmp(&A->m_info, &B->m_info, sizeof(A->m_info));

		case ACLUTIL_CXTYPE_UNIT:
			break;
	}

	return 0;
}

ae2f_inline static aclspv_wrdcount_t __find_node(const aclutil_cxtp* const ae2f_restrict rd_tar, const h_util_ctx_t h_ctx) {
	aclspv_wrdcount_t IDX = (h_ctx)->m_num_type_uniques;
	ae2f_ifezerr(h_ctx->m_type_uniques.m_p)
		return 0xFFFFFFFF;

	ae2f_ifnezerr((h_ctx)->m_num_type_uniques > (aclspv_wrd_t)(h_ctx->m_type_uniques.m_sz / sizeof(aclutil_cxtp)))
		return 0xFFFFFFFF;

	while(IDX-- 
			&& !s_ndeq(
				rd_tar
				, ((const aclutil_cxtp* ae2f_restrict)h_ctx->m_type_uniques.m_p) + IDX))
	{}

	return IDX;
}

#define	__get_type_uniques(h_ctx) ((aclutil_cxtp* ae2f_restrict)h_ctx->m_type_uniques.m_p)

ae2f_inline static struct __ret {
	aclspv_id_t	m_id; 
	aclspv_bool_t	m_isfirst; 
} s_ret(const h_aclspv_ctx_t h_ctx, const aclutil_cxtp* ae2f_restrict rd_cxtp)
{
	aclspv_wrdcount_t	IDX = __find_node(rd_cxtp, h_ctx);;
	struct	__ret		RET;

#define	_return(c_id, c_is_first)	{ RET.m_id = c_id; RET.m_isfirst = c_is_first; return RET; }

	ae2f_ifezerr_strict(IDX == 0xFFFFFFFF)
		_return(__get_type_uniques(h_ctx)[IDX].m_type_id, 0);

	IDX = h_ctx->m_num_type_uniques;
	while((IDX) >= (aclspv_wrd_t)(h_ctx->m_type_uniques.m_sz / sizeof(aclutil_cxtp))) {
		const size_t	NEWSIZE 
			= h_ctx->m_type_uniques.m_sz 
			? (h_ctx->m_type_uniques.m_sz << 1)
			: (size_t)(IDX + 1) * sizeof(aclutil_cxtp);

		_aclspv_grow_vec_with_copy(_aclspv_malloc, _aclspv_free, _aclspv_memcpy
				, L_new, h_ctx->m_type_uniques, NEWSIZE);

		ae2f_ifezerr(h_ctx->m_type_uniques.m_p) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CXTP;
			err_puts("[cxtp::s_ret] allocation has failed.");
			_return(0, 1);
		}
	}

	(void)++h_ctx->m_num_type_uniques;

	{
		const aclspv_id_t	ID = h_ctx->m_id++;
		__get_type_uniques(h_ctx)[IDX] = *rd_cxtp;
		__get_type_uniques(h_ctx)[IDX].m_type_id = ID;
		_return(ID, 1);
	}

	ae2f_unreachable();

#undef	_return
}

aclspv_id_t aclutil_mk_cxtp_arr(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		)
{
	return aclutil_mk_cxtp_arr2(
			c_type_elm, c_num_arr
			, 0xFFFFFFFF
			, h_ctx
			);
}

aclspv_id_t aclutil_mk_cxtp_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrd_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp		TAR;

	memset(&TAR, 0, sizeof(TAR));

	TAR.m_class = ACLUTIL_CXTYPE_ARRAY;
	TAR.m_info.m_arr.m_num_id = c_num_arr;
	TAR.m_info.m_arr.m_type = c_type_elm;

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeArray
					, ID.m_id
					, c_type_elm
					, c_num_arr
					)) return 0;

		if(c_decorate_stride ^ 0xFFFFFFFF)
			ae2f_expected_but_else(h_ctx->m_count.m_decorate = util_emitx_4(
						&h_ctx->m_section.m_decorate
						, h_ctx->m_count.m_decorate
						, SpvOpDecorate
						, ID.m_id
						, SpvDecorationArrayStride
						, c_decorate_stride
						)) return 0;

		return ID.m_id;
	}
}

aclspv_id_t aclutil_mk_cxtp_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t	
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp		TAR;

	memset(&TAR, 0, sizeof(TAR));

	TAR.m_class						= ACLUTIL_CXTYPE_STRUCT_SINGLE;
	TAR.m_info.m_struct_single.m_mem_id			= c_type_member;
	TAR.m_info.m_struct_single.m_decoration.m_field0	= c_decorate_field0;
	TAR.m_info.m_struct_single.m_mem_decoration.m_offset	= c_memdecorate_offset;

	(void)(aclid_mk_default_id(c_type_member, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_3(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeStruct
					, ID.m_id
					, c_type_member
					)) return 0;

		if(c_decorate_field0 & (ACLSPV_CXTP_STRUCT_DECORATION_FIELD0_BLOCK))
			ae2f_expected_but_else(h_ctx->m_count.m_decorate = util_emitx_3(
						&h_ctx->m_section.m_decorate
						, h_ctx->m_count.m_decorate
						, SpvOpDecorate
						, ID.m_id
						, SpvDecorationBlock
						)) return 0;

		if(c_memdecorate_offset ^ 0xFFFFFFFF) {
			ae2f_expected_but_else(h_ctx->m_count.m_decorate = util_emitx_5(
						&h_ctx->m_section.m_decorate
						, h_ctx->m_count.m_decorate
						, SpvOpMemberDecorate
						, ID.m_id
						, 0
						, SpvDecorationOffset
						, 0
						)) return 0;
		}

		return ID.m_id;
	}
}

aclspv_id_t aclutil_mk_cxtp_vec(
		const aclspv_id_t	c_type_elm,
		const aclspv_wrd_t	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp		TAR;

	memset(&TAR, 0, sizeof(TAR));

	TAR.m_class = ACLUTIL_CXTYPE_VECTOR;
	TAR.m_info.m_vec.m_num = c_num_arr;
	TAR.m_info.m_vec.m_type = c_type_elm;

	/** when 0, 1 arrnum is no needed or not possible */
	unless(c_num_arr >> 1) {
		return c_type_elm;
	}

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR); 
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeVector
					, ID.m_id
					, c_type_elm
					, c_num_arr
					)) return 0;

		dbg_prefix();
		dbg_call((void), (fprintf(stderr
						, "[cxtp] made a vector [%u]\n"
						, ID.m_id)));

		return ID.m_id;
	}
}

aclspv_id_t aclutil_mk_cxtp_ptr(
		const aclspv_id_t		c_type_elm,
		const enum SpvStorageClass_	c_storage_class,
		const h_aclspv_ctx_t		h_ctx
		)
{
	aclutil_cxtp		TAR;

	memset(&TAR, 0, sizeof(TAR));

	TAR.m_class			= ACLUTIL_CXTYPE_PTR;
	TAR.m_info.m_ptr.m_type		= c_type_elm;
	TAR.m_info.m_ptr.m_storage	= c_storage_class;

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR); 
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypePointer
					, ID.m_id
					, (aclspv_wrd_t) c_storage_class
					, c_type_elm
					)) return 0;

		return ID.m_id;
	}
}

aclspv_id_t aclutil_mk_cxtp_fn_no_prm(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp	TAR;
	memset(&TAR, 0, sizeof(TAR));
	TAR.m_class				= ACLUTIL_CXTYPE_FUNC_NOPRM;
	TAR.m_info.m_fn_no_proto.m_ret_id	= c_ret_id;

	(void)(aclid_mk_default_id(c_ret_id, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_3(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeFunction
					, ID.m_id
					, c_ret_id
					)) return 0;

		return ID.m_id;
	}
}

aclspv_id_t aclutil_mk_cxtp_fn_with_prm(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp	TAR;
	memset(&TAR, 0, sizeof(TAR));
	TAR.m_class				= ACLUTIL_CXTYPE_FUNC;
	TAR.m_info.m_fn.m_type			= c_type_compound;
	TAR.m_info.m_fn.m_cursor		= c_cursor_compound;

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		{
			int ARGC			= clang_getNumArgTypes(c_type_compound);
			aclspv_wrd_t	ARGC_IDX	= (aclspv_wrd_t)ARGC;
			const aclspv_wrd_t	ANCHOUR	= h_ctx->m_count.m_types;
			aclspv_wrd_t* WORDS;

			ae2f_unexpected_but_if(ARGC <= 0) {
				const CXType RET	= clang_getResultType(c_type_compound);
				const aclspv_id_t RETID	= aclutil_mk_cxtp_by_cursor(
						c_cursor_compound
						,RET 
						, h_ctx);

				ae2f_expected_but_else(RETID) return 0;
				return aclutil_mk_cxtp_fn_no_prm(RETID, h_ctx);
			}	/** fallback: argc was under 0 */

			ae2f_expected_but_else(h_ctx->m_count.m_types = util_emit_wrd(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types + (aclspv_wrd_t)ARGC + 2
						, 0)) return 0;


			WORDS = ((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_types.m_p);
			{ ae2f_assume(WORDS); }

			WORDS[ANCHOUR] = SpvOpTypeFunction;
			WORDS[ANCHOUR + 1] = ID.m_id;
			{
				const CXType RET	= clang_getResultType(c_type_compound);
				const aclspv_id_t RETID	= aclutil_mk_cxtp_by_cursor(
						c_cursor_compound
						,RET 
						, h_ctx);

				ae2f_expected_but_else(RETID) {
					h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
					return 0;
				}
				WORDS[ANCHOUR + 2] = RETID;
			}

			while(ARGC_IDX--) {
				const CXType ARG		= clang_getArgType(
						c_type_compound
						, ARGC_IDX
						);
				const aclspv_wrd_t ARG_ID	= aclutil_mk_cxtp_by_cursor(
						c_cursor_compound
						, ARG
						, h_ctx
						);

				ae2f_expected_but_else(ARG_ID)	{
					h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
					return 0;
				}

				WORDS[ARGC_IDX + ANCHOUR + 3] = ARG_ID;
			}
		}

		return ID.m_id;
	}
}


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

#include "./iddef.h"

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

ae2f_extern aclspv_id_t aclutil_mk_cxtp_by_cursor2(
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
				return aclutil_mk_cxtp_fn_with_prm(
						c_cursor
						, CANONICAL
						, h_ctx);
			}
		case CXType_FunctionNoProto:
			{
				const CXType RET	= clang_getResultType(TYPE);
				const aclspv_id_t RETID	= aclutil_mk_cxtp_by_cursor(c_cursor, RET, h_ctx);

				return aclutil_mk_cxtp_fn_no_prm(RETID, h_ctx);
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

ae2f_extern aclspv_id_t aclutil_mk_cxtp_by_cursor(
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

