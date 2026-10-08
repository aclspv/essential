#include "./body.h"
#include <aclexpr.h>
#include "util/log.h"
#include <aclcur/common.h>
#include <aclcur/ret.h>

#include <.aclspv/body/if.h>
#include <.aclspv/body/while.h>
#include <.aclspv/body/vardecl.h>
#include <.aclspv/util/mkname.h>

#include <aclspv.h>

#include <ae2f/Keys.h>
#include <ae2f/cgoto.h>

#include <util/constant.h>
#include <util/scale.h>
#include <util/literal.h>
#include <.aclspv/util/u32_to_hex8.h>

/** find the label id bound to a cursor: a defined label (LBL) or a forward-goto marker (J). */
static aclspv_id_t s_body_lbl_find(CXCursor c_cur, h_aclspv_ctx_t h_ctx) {
	aclspv_wrdcount_t i;
	aclcur_t* p_nodes = h_ctx->m_cursors.m_p;
	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		aclcur_t* p = &p_nodes[i];
		if(!(p->m_class == ACLCUR_LBL || p->m_class == ACLCUR_J))
			continue;
		if(clang_Cursor_isNull(p->m_cur))
			continue;
		if(!clang_equalCursors(p->m_cur, c_cur))
			continue;
		return p->m_class == ACLCUR_LBL ? p->m_id : p->m_d.m_j.m_lbl;
	}
	return 0;
}

enum CXChildVisitResult aclemit_entp_body(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData rdwr_data) {
#define	FNCTX		((aclutil_fnctx* ae2f_restrict) rdwr_data)
#define	CTX		FNCTX->m_ctx
#define	jmpfail(c_why)		{ (CTX)->m_err = (c_why); goto LBL_BREAK; }
#define	jmpfail_msg(rd_why)	{ err_puts(rd_why); goto LBL_FAIL; }
	const enum CXCursorKind KIND	= (h_cur).kind;
	const CXString KINDSPELL	= clang_getCursorKindSpelling(KIND);
	const CXString SPELL		= clang_getCursorSpelling(h_cur);

	ae2f_expected_but_else(KIND)
		jmpfail(ACLSPV_COMPILE_MET_INVAL_BODY_NOKIND);

	ae2f_ifnezerr(CTX->m_err) {
		goto LBL_BREAK;
	}

	FNCTX->m_state0 |= (uint_leastenum_t)(KIND != CXCursor_LabelStmt && KIND != CXCursor_ReturnStmt) << (uint_leastenum_t)(ACLUTIL_FNCTX_STATEPAD0_LBL_DO_SOMETHING);
	FNCTX->m_state0 &= ~((uint_leastenum_t)(KIND != CXCursor_ReturnStmt) << (uint_leastenum_t)(ACLUTIL_FNCTX_STATEPAD0_FNRET));

	switch((uintmax_t)KIND) {
		case CXCursor_VarDecl:
			_aclemit_body_vardecl(
					VDCL
					, h_cur
					, FNCTX
					);
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);

		case CXCursor_ReturnStmt:
			FNCTX->m_state0 |= ACLUTIL_FNCTX_STATE0_FNRET;
			FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);

			if(FNCTX->m_retid != ACLSPV_ID_VOID) {
				aclspv_id_t	RETVAL;
				struct aclattr_getkinder KINDER;
				CXCursor		CURSORCHILD;
				KINDER.m_num_wanted = 1;
				KINDER.m_num_grepped = 0;
				KINDER.m_cursor = &CURSORCHILD;
				KINDER.m_res = CXChildVisit_Continue;

				clang_visitChildren(h_cur, s_aclattr_getkinder_visit, &KINDER);

				ae2f_ifnezerr(clang_Cursor_isNull(CURSORCHILD))
					jmpfail(ACLSPV_COMPILE_UNHANDLED_BODY_CURKINDER);

				switch(aclexpr_eval(&RETVAL
							, FNCTX->m_retid
							, CURSORCHILD
							, 0
							, FNCTX)) {
					case ACLEXPR_RET_NOT_THE_CASE:
					case ACLEXPR_RET_FAILURE:
					default:
						jmpfail(ACLSPV_COMPILE_UNHANDLED_BODY_RET);

					case ACLEXPR_RET_SUCCESS:
					case ACLEXPR_RET_SUCCESS_CONSTANT:
					case ACLEXPR_RET_SUCCESS_LITERAL:
						break;
				}

				ae2f_ifezerr_strict(aclcur_cfg_ret(RETVAL, CTX)) {
					jmpfail(ACLSPV_COMPILE_UNHANDLED_BODY_RET);
				}

			} else ae2f_expected_but_else(aclcur_cfg_ret(0, CTX)) {
				jmpfail_msg("aclcur_cfg_ret\n");
			}

			ae2f_ifezerr_strict(aclcur_cfg_lbl(FNCTX->m_branch = CTX->m_id++, CTX)) {
				jmpfail(ACLSPV_COMPILE_UNHANDLED_BODY_LBL);
			}
			goto LBL_DONE;

		case CXCursor_LabelStmt:
			{
				aclspv_id_t LBL_ID = s_body_lbl_find(h_cur, CTX);

				if(!LBL_ID)
					LBL_ID = CTX->m_id++;

				ae2f_ifezerr_strict(aclcur_cfg_jmp(LBL_ID, CTX))
					jmpfail_msg("aclcur_cfg_jmp\n");
				ae2f_ifezerr_strict(aclcur_cfg_lbl(LBL_ID, CTX))
					jmpfail_msg("aclcur_cfg_lbl\n");

				FNCTX->m_branch = LBL_ID;

				ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(LBL_ID, h_cur, CTX)))
					jmpfail_msg("cursor was null (h_cur)\n");

			} ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_RECURSE, LBL_FAIL);
		case CXCursor_GotoStmt:
			{
				CXCursor LBL_STMT = clang_getCursorReferenced(h_cur);
				aclspv_id_t LBL_ID;
				aclspv_id_t JID;
				aclcur_t* JNODE;
				ae2f_expected_but_else(LBL_STMT.kind == CXCursor_LabelStmt) {
					CXString CURSORSPL = clang_getCursorDisplayName(h_cur);

					err_prefix(a);
					err_call((void), fprintf(stderr
								, "goto ref not label. Kind: %u | %s\n"
								, (unsigned)LBL_STMT.kind
								, clang_getCString(CURSORSPL)
								));

					clang_disposeString(CURSORSPL);

					jmpfail(ACLSPV_COMPILE_MET_INVAL_GOTO_NOT_LABEL);
				}

				LBL_ID = s_body_lbl_find(LBL_STMT, CTX);
				if(!LBL_ID) {
					LBL_ID = CTX->m_id++;

					JID = aclcur_cfg_jmp(LBL_ID, CTX);
					ae2f_ifezerr(JID)
						jmpfail_msg("JID\n");

					/** this is a forward-goto: mark the jump for the label. */
					JNODE = aclcur_get_node(JID, CTX);
					ae2f_ifezerr(JNODE)
						jmpfail_msg("JNODE\n");
					JNODE->m_cur = LBL_STMT;
				} else {
					ae2f_ifezerr_strict(aclcur_cfg_jmp(LBL_ID, CTX))
						jmpfail_msg("aclcur_cfg_jmp\n");
				}

				/** just for branch */
				ae2f_expected_but_else(aclcur_cfg_lbl(FNCTX->m_branch = CTX->m_id++, CTX))
					jmpfail_msg("aclcur_cfg_lbl\n");
				ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(FNCTX->m_branch, h_cur, CTX)))
					jmpfail_msg("cxcursor\n");

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
		case CXCursor_UnaryOperator:
		case CXCursor_UnaryExpr:
		case CXCursor_CallExpr:
		case CXCursor_ArraySubscriptExpr:
		case CXCursor_CStyleCastExpr:
		case CXCursor_ConditionalOperator:
		case CXCursor_DeclRefExpr:
		case CXCursor_StmtExpr:
#if 1
			{
				aclspv_id_t	DUMMY = 0;
				ae2f_ifezerr(aclexpr_eval(
							&DUMMY
							, 0
							, h_cur
							, 0
							, FNCTX))
					jmpfail_msg("aclexpr_eval\n");
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
			ae2f_ifezerr(FNCTX->m_break_loop) {
				CTX->m_err = ACLSPV_COMPILE_MET_INVAL_BRK;
				goto LBL_FAIL;
			}

			ae2f_expected_but_else(aclcur_cfg_jmp(FNCTX->m_break_loop, CTX))
				jmpfail_msg("aclcur_cfg_jmp\n");

			/** just for branch */
			ae2f_expected_but_else(aclcur_cfg_lbl(FNCTX->m_branch = CTX->m_id++, CTX))
				jmpfail_msg("aclcur_cfg_lbl\n");
			ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(FNCTX->m_branch, h_cur, CTX)))
				goto LBL_FAIL;

			FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);

		case CXCursor_ContinueStmt:
			ae2f_ifezerr(FNCTX->m_continue_loop) {
				CTX->m_err = ACLSPV_COMPILE_MET_INVAL_CONTINUE;
				goto LBL_FAIL;
			}

			ae2f_expected_but_else(aclcur_cfg_jmp(FNCTX->m_continue_loop, CTX))
				goto LBL_FAIL;

			/** just for branch */
			ae2f_expected_but_else(aclcur_cfg_lbl(FNCTX->m_branch = CTX->m_id++, CTX))
				goto LBL_FAIL;
			ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(FNCTX->m_branch, h_cur, CTX)))
				goto LBL_FAIL;

			FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);
			ae2f_cgoto2_strict(ae2f_expected_strict(!CTX->m_err), LBL_DONE, LBL_FAIL);

		case CXCursor_DoStmt:
		case CXCursor_ForStmt:

		case CXCursor_SwitchStmt:
		case CXCursor_CaseStmt:
		case CXCursor_AsmStmt: case CXCursor_MSAsmStmt:

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
	err_puts("LBL_FAIL\n");
	CTX->m_err = ae2f_bll_sel_bool_naive1(aclspv_compile_t, CTX->m_err, CTX->m_err, ACLSPV_COMPILE_UNHANDLED_BODY);

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

