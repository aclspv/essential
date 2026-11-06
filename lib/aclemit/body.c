#include "./body.h"
#include <aclexpr.h>
#include <aclcur/common.h>
#include <aclcur/ret.h>

#include <.aclspv/body/if.h>
#include <.aclspv/body/while.h>
#include <.aclspv/body/vardecl.h>
#include <.aclspv/util/mkname.h>

#include <aclspv.h>
#include <aclval.h>

#include <ae2f/Keys.h>
#include <ae2f/cgoto.h>

#include <util/scale.h>
#include <util/literal.h>
#include <.aclspv/util/u32_to_hex8.h>

#include <aclctx/err.h>

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
#define	CTX		((h_aclspv_ctx_t) rdwr_data)
#define	jmpfail(c_why)		{ _aclctx_err(CTX, (c_why)); goto LBL_BREAK; }
#define	jmpfail_msg(rd_why)	{ err_puts(rd_why); goto LBL_FAIL; }
#define	ctxcheck(h)	ae2f_ifnezerr(h->m_err) { err_puts("ctx check failed.\n"); goto LBL_FAIL; }

	const enum CXCursorKind KIND	= (h_cur).kind;
	const CXString KINDSPELL	= clang_getCursorKindSpelling(KIND);
	const CXString SPELL		= clang_getCursorSpelling(h_cur);

	/** the block opened by this statement, when it needs its own. */
	aclspv_id_t BRANCH		= 0;

	ae2f_expected_but_else(KIND)
		jmpfail(ACLSPV_COMPILE_MET_INVAL_BODY_NOKIND);

	ae2f_ifnezerr(CTX->m_err) {
		goto LBL_BREAK;
	}

	unless(KIND == CXCursor_ReturnStmt)
		aclcur_set_fnret(0, CTX);

	switch((uintmax_t)KIND) {
		case CXCursor_VarDecl:
			_aclemit_body_vardecl(
					VDCL
					, h_cur
					, CTX
					);
			ctxcheck(CTX) else goto LBL_DONE;

		case CXCursor_ReturnStmt:
			aclcur_set_fnret(1, CTX);

			if(aclcur_get_retty(CTX) != ACLSPV_ID_VOID) {
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
							, aclcur_get_retty(CTX)
							, CURSORCHILD
							, 0
							, CTX)) {
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

			ae2f_ifezerr_strict(aclcur_cfg_lbl(CTX->m_id++, CTX)) {
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

				ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(LBL_ID, h_cur, CTX)))
					jmpfail_msg("cursor was null (h_cur)\n");

			} 

			ctxcheck(CTX) else goto LBL_RECURSE;
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
				BRANCH = CTX->m_id++;
				ae2f_expected_but_else(aclcur_cfg_lbl(BRANCH, CTX))
					jmpfail_msg("aclcur_cfg_lbl\n");
				ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(BRANCH, h_cur, CTX)))
					jmpfail_msg("cxcursor\n");
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
							, CTX))
					jmpfail_msg("aclexpr_eval\n");
			}


			ctxcheck(CTX) else goto LBL_DONE;
#endif
		case CXCursor_IfStmt: 
			_aclemit_body_if(IF, h_cur, CTX);
			ctxcheck(CTX) else goto LBL_DONE;

		case CXCursor_WhileStmt:
			_aclemit_body_while(WHILE, h_cur, CTX);
			ctxcheck(CTX) else goto LBL_DONE;

		case CXCursor_BreakStmt:
			ae2f_ifezerr(aclcur_get_break_loop(CTX)) {
				jmpfail(ACLSPV_COMPILE_MET_INVAL_BRK);
			}

			ae2f_expected_but_else(aclcur_cfg_jmp(aclcur_get_break_loop(CTX), CTX))
				jmpfail_msg("aclcur_cfg_jmp\n");

			/** just for branch */
			BRANCH = CTX->m_id++;
			ae2f_expected_but_else(aclcur_cfg_lbl(BRANCH, CTX))
				jmpfail_msg("aclcur_cfg_lbl\n");
			ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(BRANCH, h_cur, CTX)))
				jmpfail_msg("aclcur_set_cxcursor\n");

			ctxcheck(CTX) else goto LBL_DONE;

		case CXCursor_ContinueStmt:
			ae2f_ifezerr(aclcur_get_continue_loop(CTX))
				jmpfail(ACLSPV_COMPILE_MET_INVAL_CONTINUE);

			ae2f_ifezerr(aclcur_cfg_jmp(aclcur_get_continue_loop(CTX), CTX))
				jmpfail_msg("aclcur_cfg_jmp\n");

			/** just for branch */
			BRANCH = CTX->m_id++;
			ae2f_ifezerr(aclcur_cfg_lbl(BRANCH, CTX))
				jmpfail_msg("aclcur_cfg_lbl\n");
			ae2f_ifnezerr(clang_Cursor_isNull(aclcur_set_cxcursor(BRANCH, h_cur, CTX)))
				jmpfail_msg("aclcur_set_cxcursor\n");

			ctxcheck(CTX) else goto LBL_DONE;

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
	_aclctx_err(CTX, ACLSPV_COMPILE_UNHANDLED_BODY);

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

