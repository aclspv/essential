#ifndef	aclemit_body_vardecl_c
#define aclemit_body_vardecl_c

#include <util/constant.h>
#include <util/fnctx.h>
#include <util/log.h>

#include <aclattr/specid.h>
#include <aclattr/btcast.h>

#include <aclexpr.h>
#include <aclexpr/curreg.h>
#include <aclcur/common.h>
#include <aclcur/reg/var.h>

#include <aclty.h>

#include <ae2f/pat/macrofunc.h>
#include <ae2f/pat/dbg.h>


/** @brief whether the declaration's own type is an array.
 *
 * an array-typed VarDecl carries its array-size expression as a child
 * cursor. that child is not an initializer, so it must not be mistaken
 * for one. note a named element type contributes a CXCursor_TypeRef child
 * of its own, which is why a `T[N]` over a builtin element type is the
 * shape that reaches this predicate unabsorbed.
 * */
static aclspv_bool_t	s_vardecl_ty_arr(CXCursor h_cur) {
	const enum CXTypeKind	KIND
		= clang_getCanonicalType(clang_getCursorType(h_cur)).kind
		;

	return	KIND == CXType_ConstantArray
		||	KIND == CXType_IncompleteArray
		||	KIND == CXType_VariableArray
		||	KIND == CXType_DependentSizedArray
		;
}

/** @brief detects an expression child on a VarDecl that the C API hid from clang_Cursor_getVarDeclInitializer (e.g. the compound-literal init shape). */
static enum CXChildVisitResult	s_vardecl_init_detect(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict rw_found) {
	const enum CXCursorKind KIND = h_cur.kind;

	unless(KIND == CXCursor_TypeRef) {
		unless(KIND >= CXCursor_FirstAttr && KIND <= CXCursor_LastAttr) {
			/** an array declaration's size expression is not an
			 * initializer, hidden or otherwise. */
			unless(s_vardecl_ty_arr(h_parent)) {
				*((int* ae2f_restrict)rw_found) = 1;
				return CXChildVisit_Break;
			}
		}
	}

	return CXChildVisit_Continue;
}


/** @brief it produces error on ctx. */
ae2f_MAC((L, )) aclemit_body_vardecl(CXCursor h_cur, aclutil_fnctx* ae2f_restrict h_fnctx) { ae2fpat_assertassume(h_fnctx); {
	h_aclspv_ctx_t	L$$CTX = h_fnctx->m_ctx;
	aclutil_literal	L$$LITERAL;
	aclspv_id_t		L$$TYPEID = 0;
	aclspv_id_t		L$$TYPE_PTR = 0;
	aclspv_id_t		L$$REG = 0;
	aclspv_id_t		L$$INIT = 0;

	const CXType	L$$TYPE = clang_getCanonicalType(clang_getCursorType(h_cur));

	L$$LITERAL.m_api_uintmax = 0;

	ae2f_ifneznrm((L$$TYPEID = aclty_mk_by_cursor_dummy(h_cur, L$$TYPE, L$$CTX))) {
		L$$TYPE_PTR = aclty_mk_ptr(
				L$$TYPEID
				, SpvStorageClassFunction
				, L$$CTX
				);

		goto L$$VDECL_TY;
	}

	else {
		CXString L$$NAME = clang_getTypeSpelling(L$$TYPE);
		err_prefix(a);
		err_call(
				fprintf, (
					stderr
					, "Met the unknown type(%u): %s\n"
					, (unsigned)L$$TYPE.kind
					, (const char*)L$$NAME.data
					);
			);
		L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL_TYPE_NOT_KNOWN;
		clang_disposeString(L$$NAME);
		goto L$$LBL_END;
	}

	L$$VDECL_TY:

		ae2f_expected_but_else(L$$TYPE_PTR)
		goto L$$LBL_END;

	/* the register is bound to its declaration cursor; loads resolve it. */
	L$$REG = aclexpr_curreg_find(h_cur, L$$CTX);
	unless(L$$REG) {
		const aclcur_reg_t L$$REGIN = aclcur_regvar_in();

		L$$REG = aclcur_cfg_reg(h_cur, L$$TYPEID, SpvStorageClassFunction, &L$$REGIN, L$$CTX);
	}

	ae2f_ifezerr(L$$REG) {
		L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
		goto L$$LBL_END;
	}

	dbg_call((void), fprintf(stderr, "[vardecl] reg %u ty %u ptr %u\n"
				, (unsigned)L$$REG
				, (unsigned)L$$TYPEID
				, (unsigned)L$$TYPE_PTR
				));

	if(aclty_is_simd(L$$TYPEID, CTX))
	{
		aclspv_id_t	L$$SPECID = 0xFFFFFFFF;

		clang_visitChildren(h_cur, s_attr_specid, &L$$SPECID);

		if(L$$SPECID != 0xFFFFFFFF) {
			/** specid! */
			aclval_spec_t	L$$NODE
				= aclval_get_spec(L$$CTX, L$$SPECID);

			dbg_prefix(i);
			dbg_call(fprintf, (stderr, "Spec ID detected: %u\n", L$$SPECID));

			if(L$$NODE.m_val) {
				dbg_prefix(a);
				dbg_call(fprintf, (stderr, "Spec ID definition spotted.: %u\n"
							, L$$NODE.m_val));

				switch(aclexpr_arithmetic_cast_non_literal(
							L$$NODE.m_val
							, L$$NODE.m_ty
							, ACLUTIL_LITERAL_CONSTANT
							, &L$$INIT
							, L$$TYPEID
							, L$$CTX
							))
				{
					case ACLEXPR_RET_SUCCESS_LITERAL:
					default:
						assert(0 && "unreachable & default");
						ae2f_unreachable();
						break;
					case ACLEXPR_RET_NOT_THE_CASE:
						assert(0 && "not the case");
						ae2f_unreachable();
						break;

					case ACLEXPR_RET_FAILURE:
					case ACLEXPR_RET_SUCCESS:
						goto L$$LBL_END;

					case ACLEXPR_RET_SUCCESS_CONSTANT:
						break;
				}

				ae2f_ifezerr(aclcur_store_reg(L$$REG, L$$INIT, L$$CTX)) {
					L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
					goto L$$LBL_END;
				}

				goto L$$LBL_END;
			}
			else {
				CXCursor	L$$VAR_INIT = clang_Cursor_getVarDeclInitializer(h_cur);
				ae2f_ifnezerr(clang_Cursor_isNull(L$$VAR_INIT)) {
					(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_BODY_VARDECL;
					goto L$$LBL_END;
				}
				else {
					fn_aclspv_get_expr_t*	L$$FN_GETEXPR = aclexpr_eval;

					{
						int L$$IS_BTCAST = 0;
						clang_visitChildren(h_cur, aclattr_btcast, &L$$IS_BTCAST);
						if(L$$IS_BTCAST) L$$FN_GETEXPR = aclexpr_eval_btcast;
					}

					ae2f_ifnezerr_strict(ACLEXPR_RET_SUCCESS_LITERAL != L$$FN_GETEXPR(
								&L$$INIT
								, L$$TYPEID
								, L$$VAR_INIT
								, &L$$LITERAL
								, h_fnctx)) {
						L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL_BODY_VARDECL_GET_EXPR;
						goto L$$LBL_END;
					}

					ae2f_ifezerr(L$$INIT = aclutil_mk_specconstant_val_by_enum(L$$SPECID
								, L$$LITERAL.m_api_uintmax
								, L$$CTX
								, (enum ACLSPV_ID_) L$$TYPEID)) {
						L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
						goto L$$LBL_END;
					}

					ae2f_ifezerr(aclcur_store_reg(L$$REG, L$$INIT, L$$CTX)) {
						L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
						goto L$$LBL_END;
					}

					goto L$$LBL_END;
				}
			}
		}
	}

	{
		CXCursor	L$$VAR_INIT = clang_Cursor_getVarDeclInitializer(h_cur);
		if(clang_Cursor_isNull(L$$VAR_INIT)) {
			/** the C API hid an existing initializer (e.g. the compound-literal shape):
			 * 		fail loudly instead of registering an uninitialized variable. */
			int	L$$HAS_INIT = 0;

			clang_visitChildren(h_cur, s_vardecl_init_detect, &L$$HAS_INIT);

			ae2f_ifnezerr(L$$HAS_INIT) {
				err_puts("[aclemit::body::vardecl] initializer exists but is invisible to clang_Cursor_getVarDeclInitializer (compound-literal init?) — not supported");
				L$$CTX->m_err = ACLSPV_COMPILE_UNHANDLED_BODY_VARDECL;
			}
			goto L$$LBL_END;
		}

		dbg_prefix(a);
		dbg_call(fprintf, (stderr, "[vardecl]: VAR_INIT.kind: %d\n", (int)L$$VAR_INIT.kind));	
		dbg_puts(("Variable has constructor.\n"));

		{
			fn_aclspv_get_expr_t*	L$$FN_GETEXPR;
			aclspv_bool_t L$$IS_BTCAST = 0;
			clang_visitChildren(h_cur, aclattr_btcast, &L$$IS_BTCAST);
			L$$FN_GETEXPR = L$$IS_BTCAST ? aclexpr_eval_btcast : aclexpr_eval;

			switch(L$$FN_GETEXPR(
						&L$$INIT
						, L$$TYPEID
						, L$$VAR_INIT
						, &L$$LITERAL
						, h_fnctx))
			{
				case ACLEXPR_RET_NOT_THE_CASE:
					L$$CTX->m_err = ACLSPV_COMPILE_NOIMPL;
					break;

				default:
				case ACLEXPR_RET_FAILURE:
					unless(L$$CTX->m_err)
						L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL_BODY_VARDECL_GET_EXPR;
					break;

				case ACLEXPR_RET_SUCCESS_LITERAL:
				case ACLEXPR_RET_SUCCESS_CONSTANT:
				case ACLEXPR_RET_SUCCESS:
					break;
			}
		}

		ae2f_ifnezerr(L$$CTX->m_err)
			goto L$$LBL_END;

		ae2f_ifezerr(aclcur_store_reg(L$$REG, L$$INIT, L$$CTX)) {
			L$$CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
			goto L$$LBL_END;
		}
	}

} L$$LBL_END: (void)0; }


#endif
