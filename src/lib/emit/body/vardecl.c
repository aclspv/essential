#ifndef	aclemit_body_vardecl_c
#define aclemit_body_vardecl_c

#include <util/cxtp.h>
#include <util/cursor.h>
#include <util/constant.h>
#include <util/fnctx.h>
#include <util/log.h>

#include <attr/specid.h>
#include <attr/btcast.h>
#include <attr/vecdecl.h>

#include <emit/expr.h>

#include <ae2f/pat/macrofunc.h>
#include <ae2f/pat/dbg.h>


/** @brief it produces error on ctx. */
ae2f_MAC((L, )) aclemit_body_vardecl(CXCursor h_cur, aclutil_fnctx* ae2f_restrict h_fnctx) { ae2fpat_assertassume(h_fnctx); {
	h_aclspv_ctx_t	L$$CTX = h_fnctx->m_ctx;

	aclspv_wrdcount_t L$$IDX = aclutil_mk_cursor_base(
			L$$CTX->m_num_cursor
			, &L$$CTX->m_cursors
			, h_cur);

	aclspv_id_t		L$$TYPE_PTR = 0, L$$TYPE_VEC = 0;

	aclspv_wrdcount_t	L$$ORIGIN_CURSORS_NUM = L$$CTX->m_num_cursor;

	const CXType	L$$TYPE = clang_getCanonicalType(clang_getCursorType(h_cur));
	util_cursor* L$$PCURSOR = &((util_cursor* ae2f_restrict)L$$CTX->m_cursors.m_p)[L$$IDX];

	const aclspv_wrd_t	L$$VECNUM_	= aclattr_vecdecl(0, h_cur);
	const aclspv_wrd_t	L$$VECNUM	= L$$VECNUM_ ? L$$VECNUM_ : aclattr_vecdecl(1
			, clang_getTypeDeclaration(clang_getCursorType(h_cur)));

	ae2f_unexpected_but_if(L$$IDX == L$$CTX->m_num_cursor + 1)
		goto L$$LBL_END;

	L$$CTX->m_num_cursor += L$$IDX == L$$CTX->m_num_cursor;

	ae2f_ifnezerr(0) {	/** arithmetic */
		ae2f_unreachable();
		L$$ARITHMETIC_VEC:
		L$$TYPE_PTR = aclutil_mk_cxtp_ptr(
				L$$PCURSOR->m_data.m_vardecl.m_type_id = L$$TYPE_VEC
				, SpvStorageClassFunction
				, L$$CTX
				);

		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "etype: %u type: %u\n"
				, (unsigned)L$$PCURSOR->m_data.m_vardecl.m_etype_id
				, (unsigned)L$$PCURSOR->m_data.m_vardecl.m_type_id
			)));

		goto L$$VDECL_TY;
	}

	switch((uintmax_t)L$$TYPE.kind) {
		case CXType_UInt:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_U32, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_U32
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

		case CXType_UShort:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_U16, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_U16
					, L$$VECNUM
					, L$$CTX);


			goto L$$ARITHMETIC_VEC;

		case CXType_Char_U:
		case CXType_UChar:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_U8, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_U8
					, L$$VECNUM
					, L$$CTX);


			goto L$$ARITHMETIC_VEC;

		case CXType_Int:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_I32, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_I32
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

		case CXType_Short:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_I16, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_I16
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

		case CXType_SChar:
		case CXType_Char_S:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_I8, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_I8
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

		case CXType_ULongLong:
		case CXType_ULong:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_U64, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_U64
					, L$$VECNUM
					, L$$CTX);


			goto L$$ARITHMETIC_VEC;

		case CXType_LongLong:
		case CXType_Long:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_I64, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_I64
					, L$$VECNUM
					, L$$CTX);


			goto L$$ARITHMETIC_VEC;

		case CXType_BFloat16:
		case CXType_Float16:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_F16, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_F16
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

		case CXType_Float:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_F32, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_F32
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

		case CXType_Double:
			ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_F64, L$$CTX))
				goto L$$LBL_END;

			L$$TYPE_VEC = aclutil_mk_cxtp_vec(
					L$$PCURSOR->m_data.m_vardecl.m_etype_id = ID_DEFAULT_F64
					, L$$VECNUM
					, L$$CTX);

			goto L$$ARITHMETIC_VEC;

			/** complex types. we make it later. */

		case CXType_Record:
			{
				const intmax_t	L$$TY_SIZEOF_ = clang_Type_getSizeOf(L$$TYPE);
				const uintmax_t	 L$$TY_SIZEOF = L$$TY_SIZEOF_ < 0 ? 0 : (uintmax_t)L$$TY_SIZEOF_;
				const aclspv_wrd_t	 L$$TY_SIZE_WRD = L$$TY_SIZEOF < UINT32_MAX 
					? (aclspv_wrd_t)L$$TY_SIZEOF : 0;

				ae2f_ifezerr(L$$TY_SIZE_WRD) {
					L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
					goto L$$LBL_END;
				}

				ae2f_expected_but_else(L$$TYPE_PTR = aclutil_mk_constant_ptr_func(
							(L$$TY_SIZE_WRD + 3) >> 2
							, L$$CTX))
					goto L$$LBL_END;
				L$$PCURSOR->m_data.m_vardecl.m_type_id = aclutil_mk_constant_structpriv_id(
						(L$$TY_SIZE_WRD + 3) >> 2, L$$CTX
						);
			}
			break;

		default:
			{
				CXString L$$NAME = clang_getTypeSpelling(L$$TYPE);
				err_prefix(a);
				err_call(
						fprintf, (
							stderr
							, "Met the unknown type(%u): %s\n"
							, L$$TYPE.kind
							, (const char*)L$$NAME.data
							);
					)
					clang_disposeString(L$$NAME);
			}
			break;
	}

	L$$VDECL_TY:

	ae2f_expected_but_else(L$$TYPE_PTR)
		goto L$$LBL_END;

	L$$PCURSOR->m_data.m_vardecl.m_ptr_type_id = L$$TYPE_PTR;
	L$$PCURSOR->m_data.m_vardecl.m_id = L$$CTX->m_id++;

	L$$PCURSOR->m_data.m_vardecl.m_mask = 0;
	L$$PCURSOR->m_data.m_vardecl.m_stage_lvl = h_fnctx->m_var_stage;

	if(aclid_is_arithmetic(L$$PCURSOR->m_data.m_vardecl.m_type_id, L$$CTX)) {
		aclspv_wrdcount_t L$$STAGEDIDX = aclutil_from_cursor_find_stagedval(L$$ORIGIN_CURSORS_NUM
				, &h_fnctx->m_ctx->m_cursors
				, L$$PCURSOR->m_data.m_vardecl.m_stage_lvl
				, L$$TYPE_PTR);

		dbg_puts("we will find owner");

		if(L$$STAGEDIDX < L$$ORIGIN_CURSORS_NUM) {
			(void)L$$CTX->m_id;
			L$$PCURSOR->m_data.m_vardecl.m_mask &= ~(aclspv_wrd_t)ACLUTIL_CURSOR_VAR_OWNER;
			L$$PCURSOR->m_data.m_vardecl.m_id
				= L$$PCURSOR[L$$STAGEDIDX - L$$IDX].m_data.m_vardecl.m_id;
			L$$PCURSOR[L$$STAGEDIDX - L$$IDX].m_data.m_vardecl.m_mask |= ACLUTIL_CURSOR_VAR_OCCUPIED;
		} else {
			L$$PCURSOR->m_data.m_vardecl.m_mask |= ACLUTIL_CURSOR_VAR_OWNER;
			dbg_puts("is the owner");
		}

		L$$PCURSOR->m_data.m_vardecl.m_mask |= ACLUTIL_CURSOR_VAR_OCCUPIED;
	}

	if(aclid_is_arithmetic(L$$PCURSOR->m_data.m_vardecl.m_type_id, L$$CTX))
	{
		CXCursor	L$$VAR_INIT = clang_Cursor_getVarDeclInitializer(h_cur);
		aclspv_id_t	L$$SPECID = 0xFFFFFFFF;

		clang_visitChildren(h_cur, s_attr_specid, &L$$SPECID);

		if(L$$SPECID != 0xFFFFFFFF) {
			/** specid! */
			util_constant* ae2f_restrict const L$$NODE
				= aclutil_mk_constant_node(L$$SPECID, L$$CTX);

			dbg_prefix(i);
			dbg_call(fprintf, (stderr, "Spec ID detected: %u\n", L$$SPECID));

			if(L$$NODE && L$$NODE->m_const_spec_id) {
				dbg_prefix(a);
				dbg_call(fprintf, (stderr, "Spec ID definition spotted.: %u\n"
							, L$$NODE->m_const_spec_id));

				switch(aclexpr_arithmetic_cast_non_literal(
							L$$NODE->m_const_spec_id
							, L$$NODE->m_const_spec_type
							, UTIL_LITERAL_CONSTANT
							, &L$$PCURSOR->m_data.m_vardecl.m_init_val
							, L$$PCURSOR->m_data.m_vardecl.m_type_id
							, L$$CTX
							))
				{
					case EMIT_EXPR_SUCCESS_LITERAL:
					default:
						assert(0 && "unreachable & default");
						ae2f_unreachable();
						break;
					case EMIT_EXPR_NOT_THE_CASE:
						assert(0 && "not the case");
						ae2f_unreachable();
						break;

					case EMIT_EXPR_FAILURE:
						goto L$$LBL_END;
						ae2f_unexpected_but_if(0) {
							ae2f_unreachable();
							case EMIT_EXPR_SUCCESS:
							L$$PCURSOR->m_data.m_vardecl.m_mask
								&= (aclspv_wrd_t)~ACLUTIL_CURSOR_VAR_LITERAL;
						}

						ae2f_unexpected_but_if(0) {
							ae2f_unreachable();
							case EMIT_EXPR_SUCCESS_CONSTANT:
							L$$PCURSOR->m_data.m_vardecl.m_mask
								&= ~(aclspv_wrd_t)ACLUTIL_CURSOR_VAR_LITERAL | UTIL_LITERAL_CONSTANT;
						}

						L$$PCURSOR->m_data.m_vardecl.m_mask |= ACLUTIL_CURSOR_VAR_PREDICTABLE;
				}
			}


			goto L$$LBL_END;
		}

		unless(clang_Cursor_isNull(L$$VAR_INIT)) {
			fn_aclspv_get_expr_t*	L$$FN_GETEXPR = aclspv_get_expr;

			{
				int L$$IS_BTCAST = 0;
				clang_visitChildren(h_cur, s_attr_btcast, &L$$IS_BTCAST);
				if(L$$IS_BTCAST) L$$FN_GETEXPR = aclspv_get_expr_btcast;
			}

			dbg_puts(("Variable has constructor."));

			switch(L$$FN_GETEXPR(
						&L$$PCURSOR->m_data.m_vardecl.m_init_val
						, L$$PCURSOR->m_data.m_vardecl.m_type_id
						, L$$VAR_INIT
						, &L$$PCURSOR->m_data.m_vardecl.m_literal
						, L$$CTX))
			{
				case EMIT_EXPR_NOT_THE_CASE:
					L$$CTX->m_err = ACLSPV_COMPILE_NO_IMPL;
					break;

				default:
				case EMIT_EXPR_FAILURE:
					L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
					break;

				case EMIT_EXPR_SUCCESS_LITERAL:
					L$$PCURSOR->m_data.m_vardecl.m_mask
						|= UTIL_LITERAL_MASK_LITERAL;

					dbg_puts(("Constructor is literal."));
					ae2f_fallthrough;
				case EMIT_EXPR_SUCCESS_CONSTANT:

					dbg_puts(("Constructor may be constant."));
					L$$PCURSOR->m_data.m_vardecl.m_mask
						|= UTIL_LITERAL_CONSTANT;
					ae2f_fallthrough;
				case EMIT_EXPR_SUCCESS:
					L$$PCURSOR->m_data.m_vardecl.m_mask |= ACLUTIL_CURSOR_VAR_PREDICTABLE;
					break;
			}
		}
	}

} L$$LBL_END: (void)0; }


#endif
