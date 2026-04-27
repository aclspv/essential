#include "./entp_body.h"
#include "./expr.h"

#include <.aclspv/emit/body/if.h>
#include <.aclspv/emit/body/while.h>
#include <.aclspv/emit/body/vardecl.h>
#include <.aclspv/util/mkname.h>

#include <aclspv.h>

#include <ae2f/Keys.h>
#include <ae2f/c90/StdInt.h>
#include <ae2f/c90/StdBool.h>
#include <ae2f/cgoto.h>

#include <clang-c/CXString.h>

#include <util/constant.h>
#include <util/scale.h>
#include <util/literal.h>
#include <util/u32_to_hex8.auto.h>

#include <spirv/unified1/spirv.h>

enum CXChildVisitResult aclemit_entp_body(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData rdwr_data) {
#define	FNCTX		((aclutil_fnctx* ae2f_restrict) rdwr_data)
#define	CTX		FNCTX->m_ctx
#define	jmpfail(c_why)	{ (CTX)->m_err = (c_why); goto LBL_BREAK; }
	const enum CXCursorKind KIND	= (h_cur).kind;
	const CXString KINDSPELL	= clang_getCursorKindSpelling(KIND);
	const CXString SPELL		= clang_getCursorSpelling(h_cur);

	ae2f_expected_but_else(KIND)	{
		CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
		goto LBL_FAIL;
	}

	ae2f_ifnezerr(CTX->m_err) {
		goto LBL_BREAK;
	}

	FNCTX->m_state0 |= (uint_leastenum_t)(KIND != CXCursor_LabelStmt && KIND != CXCursor_ReturnStmt) << (uint_leastenum_t)(ACLUTIL_FNCTX_STATEPAD0_LBL_DO_SOMETHING);
	FNCTX->m_state0 &= ~((uint_leastenum_t)(KIND != CXCursor_ReturnStmt) << (uint_leastenum_t)(ACLUTIL_FNCTX_STATEPAD0_FNRET));

	switch((uintmax_t)KIND) {
		case CXCursor_VarDecl:
			_aclemit_body_vardecl(VDCL, h_cur, FNCTX);
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);

		case CXCursor_ReturnStmt:
			if(FNCTX->m_state0 & (uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING)) {
				aclutil_tell_cursor_lbl(CTX->m_num_cursor, CTX->m_cursors.m_p
						, CTX, FNCTX->m_var_stage + 1);
			} else {
				CTX->m_count.m_fnimpl = FNCTX->m_lbl_snapshot;
			}

			ae2f_expected_but_else(CTX->m_count.m_fnimpl = emit_opcode(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpReturn, 0
						)) goto LBL_FAIL;
			FNCTX->m_state0 |= ACLUTIL_FNCTX_STATE0_FNRET;
			FNCTX->m_lbl_snapshot = CTX->m_count.m_fnimpl;

			if(FNCTX->m_retid != ID_DEFAULT_VOID) {
				/** have to return a value */
			}

			FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);

			/** just for branch */
			ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpLabel
						, CTX->m_id++
						)) goto LBL_FAIL;

			goto LBL_DONE;

		case CXCursor_LabelStmt:
			unless(FNCTX->m_state0 & (uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING)) {
				CTX->m_count.m_fnimpl = FNCTX->m_lbl_snapshot;
			}

			ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
						CTX->m_num_cursor
						, CTX->m_cursors.m_p
						, CTX
						, FNCTX->m_var_stage + 1))
				goto LBL_FAIL;
			{

#define	CURSOR	((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
				const aclspv_wrd_t IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, h_cur);


				ae2f_unexpected_but_if(CTX->m_num_cursor + 1 == IDX)
					goto LBL_FAIL;

				if(CTX->m_num_cursor == IDX) {
					CURSOR.m_data.m_goto_lbl.m_id = CTX->m_id++;
					++CTX->m_num_cursor;
				}

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpBranch
							, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
							.m_data.m_goto_lbl.m_id
							)) goto LBL_FAIL;

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpLabel
							, CURSOR.m_data.m_goto_lbl.m_id
							)) goto LBL_FAIL;

				_aclutil_mkname_dbg(L, CURSOR.m_data.m_goto_lbl.m_id, SPELL.data, *CTX, CTX->m_tmp.m_v0);

#undef	CURSOR
			} ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_RECURSE, LBL_FAIL);
		case CXCursor_GotoStmt:
			{
				CXCursor LBL_STMT = clang_getCursorReferenced(h_cur);
				aclspv_wrdcount_t IDX;
				assert(LBL_STMT.kind == CXCursor_LabelStmt);
				ae2f_expected_but_else(LBL_STMT.kind == CXCursor_LabelStmt)
					jmpfail(ACLSPV_COMPILE_MET_INVAL);

				IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, LBL_STMT
						);

				ae2f_unexpected_but_if(CTX->m_num_cursor + 1 == IDX) goto LBL_FAIL;
				if(CTX->m_num_cursor == IDX) {
					((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_cursor
						= LBL_STMT;
					((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_data.m_goto_lbl.m_id
						= CTX->m_id++;

					++CTX->m_num_cursor;
				}


				/** snapshot */
				FNCTX->m_lbl_snapshot	= CTX->m_count.m_fnimpl;

				if(FNCTX->m_state0 & (uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING)) {
					aclutil_tell_cursor_lbl(CTX->m_num_cursor, CTX->m_cursors.m_p
							, CTX, FNCTX->m_var_stage + 1);
				}

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpBranch
							, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
							.m_data.m_goto_lbl.m_id
							)) goto LBL_FAIL;

				/** just for branch */
				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpLabel
							, CTX->m_id++
							)) goto LBL_FAIL;

				FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);
			}
			goto LBL_DONE;

			/** 
			 * if this is surely just {},
			 * 	then it is just another block ready to be merged. 
			 * */
		case CXCursor_CompoundStmt:
			goto LBL_RECURSE;

		case CXCursor_BinaryOperator:
		case CXCursor_CompoundAssignOperator:
#if 1
			{
				const aclspv_wrdcount_t EXPR_IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, h_cur
						);

				aclspv_id_t	CONDVALID = CTX->m_id++;

				ae2f_ifezerr(EXPR_IDX != CTX->m_num_cursor + 1)
					goto LBL_FAIL;

				CTX->m_num_cursor += EXPR_IDX == CTX->m_num_cursor;

				ae2f_ifezerr(aclspv_get_expr(
							&CONDVALID
							, 0
							, h_cur
							, 0
							, CTX))
					goto LBL_FAIL;
			}
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);
#endif
		case CXCursor_IfStmt: 
			_aclemit_body_if(IF, h_cur, FNCTX);
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);

		case CXCursor_WhileStmt:
			_aclemit_body_while(WHILE, h_cur, FNCTX);
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);

		case CXCursor_BreakStmt:
		case CXCursor_ContinueStmt:

		case CXCursor_UnaryOperator:
		case CXCursor_CallExpr:
		case CXCursor_ArraySubscriptExpr:
		case CXCursor_ConditionalOperator:
		case CXCursor_CStyleCastExpr:
		case CXCursor_DeclRefExpr:
		case CXCursor_UnaryExpr:
		case CXCursor_DoStmt:
		case CXCursor_ForStmt:

		case CXCursor_SwitchStmt:
		case CXCursor_CaseStmt:
		case CXCursor_AsmStmt: case CXCursor_MSAsmStmt:

		case CXCursor_StmtExpr:
		default:
			break;
	}


	warn_prefix(i);
	warn_call(fprintf, (stderr, "Met unprocessed spell(%s) in kind(%s) [NOIMPL, BODY]\n"
				, (const char*)SPELL.data
				, (const char*)KINDSPELL.data));

LBL_RECURSE:
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Recurse;

	ae2f_unreachable();
LBL_FAIL:
	CTX->m_err = ae2f_bll_sel_bool_naive1(e_aclspv_compile_t, CTX->m_err, CTX->m_err, ACLSPV_COMPILE_UNHANDLED_BODY);

LBL_BREAK:
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Break;

	ae2f_unreachable();
LBL_DONE:
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Continue;

#undef	CTX
#undef	jmpfail
}

