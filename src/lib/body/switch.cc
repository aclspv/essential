#ifndef	aclemit_body_switch_cc
#define aclemit_body_switch_cc

#include <aclattr/getkinder.h>

#include <aclexpr.h>
#include <aclcur/common.h>
#include <aclval.h>
#include <acltrl.h>
#include <aclmem.h>
#include <aclctx/id.h>

#include <aclemit/body.h>
#include <aclutil/emitx.h>
#include <util/visitfromroot.h>

#include <ae2f/cc.h>
#include <ae2f/pat/macrofunc.h>

#include <.aclspv/util/mkname.h>

#include <string.h>

/** one parsed case entry lives in aclcur_sw_entry_t (val + group label). */

typedef struct s_aclemit_switch_collect {
	h_aclspv_ctx_t	m_ctx;

	/** aclcur_sw_entry_t entries, one per `case`. */
	x_aclspv_vec	m_cases;

	/** aclspv_id_t, one per case group (a run of empty labels shares one). */
	x_aclspv_vec	m_groups;

	aclspv_id_t	m_default;
	aclspv_bool_t	m_has_default;
	aclspv_bool_t	m_err;
} s_aclemit_switch_collect_t;

typedef struct s_aclemit_switch_emit {
	h_aclspv_ctx_t		m_ctx;
	aclemit_body_t*		m_body;
	const aclspv_id_t*	m_groups;
	aclspv_wrd_t		m_num_groups;

	/** index of the currently open group. */
	aclspv_wrd_t		m_group;
	/** index of the next group to open. */
	aclspv_wrd_t		m_next;
	aclspv_bool_t		m_open;

	aclspv_id_t		m_merge;
	aclspv_bool_t		m_err;
} s_aclemit_switch_emit_t;

typedef struct s_aclemit_switch_children {
	CXCursor*	m_p;
	aclspv_wrd_t	m_max;
	aclspv_wrd_t	m_n;
} s_aclemit_switch_children_t;

static enum CXChildVisitResult s_aclemit_switch_child_visit(
		CXCursor c_cur,
		ae2f_unused CXCursor c_par,
		CXClientData rdwr_data) {
	s_aclemit_switch_children_t* ae2f_restrict p = rdwr_data;

	if(p->m_n < p->m_max)
		p->m_p[p->m_n++] = c_cur;

	return p->m_n >= p->m_max ? CXChildVisit_Break : CXChildVisit_Continue;
}

/** direct children of c_cur, at most c_max. */
static void s_aclemit_switch_children(
		CXCursor	c_cur,
		CXCursor*	rd_p,
		aclspv_wrd_t	c_max,
		aclspv_wrd_t*	wr_n) {
	s_aclemit_switch_children_t S;

	S.m_p	= rd_p;
	S.m_max	= c_max;
	S.m_n	= 0;

	(void)clang_visitChildren(c_cur, s_aclemit_switch_child_visit, &S);
	*wr_n = S.m_n;
}

static aclspv_bool_t s_aclemit_switch_case_push(
		s_aclemit_switch_collect_t* ae2f_restrict p,
		aclspv_wrd2_t c_val,
		aclspv_id_t c_lbl) {
	aclspv_wrd_t n = ae2f_static_cast(aclspv_wrd_t,
			p->m_cases.m_sz / ae2f_static_cast(aclspv_wrd_t, sizeof(aclcur_sw_entry_t)));

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&p->m_cases
				, n + 1u
				, ae2f_static_cast(aclspv_wrd_t, sizeof(aclcur_sw_entry_t))
				, ACLSPV_COMPILE_ALLOC_FAILED_VEC))
		return 0;

	{
		aclcur_sw_entry_t* const E =
			ae2f_reinterpret_cast(aclcur_sw_entry_t*, p->m_cases.m_p);

		E[n].m_val = c_val;
		E[n].m_lbl = c_lbl;
	}

	return 1;
}

static aclspv_bool_t s_aclemit_switch_group_push(
		s_aclemit_switch_collect_t* ae2f_restrict p,
		aclspv_id_t c_lbl) {
	aclspv_wrd_t n = ae2f_static_cast(aclspv_wrd_t,
			p->m_groups.m_sz / ae2f_static_cast(aclspv_wrd_t, sizeof(aclspv_id_t)));

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&p->m_groups
				, n + 1u
				, ae2f_static_cast(aclspv_wrd_t, sizeof(aclspv_id_t))
				, ACLSPV_COMPILE_ALLOC_FAILED_VEC))
		return 0;

	ae2f_reinterpret_cast(aclspv_id_t*, p->m_groups.m_p)[n] = c_lbl;
	return 1;
}

/** evaluate a case's constant expression. */
static aclspv_bool_t s_aclemit_switch_eval_case(
		CXCursor	c_cur,
		aclspv_wrd2_t*	wr_val) {
	CXEvalResult E = clang_Cursor_Evaluate(c_cur);
	aclspv_swrd2_t V;

	if(!E)
		return 0;

	if(clang_EvalResult_getKind(E) != CXEval_Int) {
		clang_EvalResult_dispose(E);
		return 0;
	}

	V = ae2f_static_cast(aclspv_swrd2_t, clang_EvalResult_getAsLongLong(E));
	clang_EvalResult_dispose(E);

	*wr_val = ae2f_static_cast(aclspv_wrd2_t, V);
	return 1;
}

/** follow a `case`/`default` chain. empty labels (`case 1: case 2:`) nest
 * the next label as the sub-statement; all of them share the group label,
 * and the first real statement is the group's first statement.
 *
 * returns 0 on a malformed / unsupported form. */
static aclspv_bool_t s_aclemit_switch_collect_chain(
		CXCursor			c_cur,
		aclspv_id_t			c_lbl,
		s_aclemit_switch_collect_t*	rdwr_p) {
	while(!clang_Cursor_isNull(c_cur)) {
		CXCursor		CH[2];
		CXCursor		NXT = clang_getNullCursor();
		aclspv_wrd_t		N = 0;
		const enum CXCursorKind KIND = clang_getCursorKind(c_cur);

		if(KIND != CXCursor_CaseStmt && KIND != CXCursor_DefaultStmt)
			return 1;

		s_aclemit_switch_children(c_cur, CH, 2, &N);

		if(KIND == CXCursor_CaseStmt) {
			aclspv_wrd2_t VAL = 0;

			if(N < 1 || !s_aclemit_switch_eval_case(CH[0], &VAL))
				return 0;
			if(!s_aclemit_switch_case_push(rdwr_p, VAL, c_lbl))
				return 0;
			if(N >= 2)
				NXT = CH[1];
		} else {
			if(rdwr_p->m_has_default)
				return 0;
			rdwr_p->m_has_default	= 1;
			rdwr_p->m_default	= c_lbl;
			if(N >= 1)
				NXT = CH[0];
		}

		if(clang_Cursor_isNull(NXT)
				|| (clang_getCursorKind(NXT) != CXCursor_CaseStmt
					&& clang_getCursorKind(NXT) != CXCursor_DefaultStmt))
			return 1;

		c_cur = NXT;
	}

	return 1;
}

static enum CXChildVisitResult s_aclemit_switch_collect_visit(
		CXCursor c_cur,
		ae2f_unused CXCursor c_par,
		CXClientData rdwr_data) {
	s_aclemit_switch_collect_t* ae2f_restrict p = rdwr_data;
	const enum CXCursorKind KIND = clang_getCursorKind(c_cur);
	aclspv_id_t LBL;

	if(KIND != CXCursor_CaseStmt && KIND != CXCursor_DefaultStmt)
		return CXChildVisit_Continue;

	LBL = aclctx_newid(p->m_ctx);
	if(!LBL
			|| !s_aclemit_switch_group_push(p, LBL)
			|| !s_aclemit_switch_collect_chain(c_cur, LBL, p)) {
		p->m_err = 1;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}

static aclspv_bool_t s_aclemit_switch_emit_stmt(
		CXCursor			c_cur,
		s_aclemit_switch_emit_t*	rdwr_p) {
	if(clang_Cursor_isNull(c_cur)
			|| clang_getCursorKind(c_cur) == CXCursor_NullStmt)
		return 1;

	(void)aclutil_visitfromroot(c_cur, aclemit_body, rdwr_p->m_body);

	return (aclspv_bool_t)!rdwr_p->m_ctx->m_err;
}

/** emit the first real statement of the group, skipping the nested labels. */
static aclspv_bool_t s_aclemit_switch_emit_chain(
		CXCursor			c_cur,
		s_aclemit_switch_emit_t*	rdwr_p) {
	while(!clang_Cursor_isNull(c_cur)) {
		CXCursor		CH[2];
		CXCursor		NXT = clang_getNullCursor();
		aclspv_wrd_t		N = 0;
		const enum CXCursorKind KIND = clang_getCursorKind(c_cur);

		if(KIND != CXCursor_CaseStmt && KIND != CXCursor_DefaultStmt)
			return s_aclemit_switch_emit_stmt(c_cur, rdwr_p);

		s_aclemit_switch_children(c_cur, CH, 2, &N);

		if(KIND == CXCursor_CaseStmt) {
			if(N >= 2)
				NXT = CH[1];
		} else if(N >= 1) {
			NXT = CH[0];
		}

		if(clang_Cursor_isNull(NXT)
				|| (clang_getCursorKind(NXT) != CXCursor_CaseStmt
					&& clang_getCursorKind(NXT) != CXCursor_DefaultStmt))
			return s_aclemit_switch_emit_stmt(NXT, rdwr_p);

		c_cur = NXT;
	}

	return 1;
}

static aclspv_bool_t s_aclemit_switch_open(
		s_aclemit_switch_emit_t*	rdwr_p,
		aclspv_wrd_t			c_idx) {
	const aclspv_id_t LBL = rdwr_p->m_groups[c_idx];

	aclcur_set_fnret(0, rdwr_p->m_ctx);

	ae2f_ifezerr(aclcur_cfg_lbl(LBL, rdwr_p->m_ctx))
		return 0;

	rdwr_p->m_group	= c_idx;
	rdwr_p->m_open	= 1;

	aclutil_mkname_dbg_fn(
			LBL
			, "case"
			, rdwr_p->m_ctx
			, &rdwr_p->m_ctx->m_tmp.m_v0);

	return (aclspv_bool_t)!rdwr_p->m_ctx->m_err;
}

/** close the open group: the fall-through jump goes to the next group, or
 * to the merge for the last one. a group that ended in a return needs none. */
static aclspv_bool_t s_aclemit_switch_close(s_aclemit_switch_emit_t* rdwr_p) {
	aclspv_id_t TAR;

	unless(rdwr_p->m_open)
		return 1;

	rdwr_p->m_open = 0;

	unless(aclcur_get_fnret(rdwr_p->m_ctx)) {
		TAR = rdwr_p->m_group + 1u < rdwr_p->m_num_groups
			? rdwr_p->m_groups[rdwr_p->m_group + 1u]
			: rdwr_p->m_merge;

		ae2f_ifezerr(aclcur_cfg_jmp(TAR, rdwr_p->m_ctx))
			return 0;
	}

	return 1;
}

static enum CXChildVisitResult s_aclemit_switch_emit_visit(
		CXCursor c_cur,
		ae2f_unused CXCursor c_par,
		CXClientData rdwr_data) {
	s_aclemit_switch_emit_t* ae2f_restrict p = rdwr_data;
	const enum CXCursorKind KIND = clang_getCursorKind(c_cur);

	if(KIND == CXCursor_CaseStmt || KIND == CXCursor_DefaultStmt) {
		unless(s_aclemit_switch_close(p)) {
			p->m_err = 1;
			return CXChildVisit_Break;
		}

		if(p->m_next >= p->m_num_groups
				|| !s_aclemit_switch_open(p, p->m_next)) {
			p->m_err = 1;
			return CXChildVisit_Break;
		}

		(void)++p->m_next;

		if(!s_aclemit_switch_emit_chain(c_cur, p)) {
			p->m_err = 1;
			return CXChildVisit_Break;
		}
	} else if(p->m_open) {
		if(!s_aclemit_switch_emit_stmt(c_cur, p)) {
			p->m_err = 1;
			return CXChildVisit_Break;
		}
	}

	return CXChildVisit_Continue;
}

ae2f_MAC((L, )) aclemit_body_switch(CXCursor h_cur, aclemit_body_t& rdwr_body) {
	struct aclattr_getkinder	L$$KINDER;
	CXCursor			L$$CUR[2];

	s_aclemit_switch_collect_t	L$$COL;
	s_aclemit_switch_emit_t		L$$EMIT;

	aclspv_id_t			L$$SEL = 0,
					L$$MERGE,
					L$$DEFAULT,
					L$$SW,
					L$$BRK_PAR;
	aclty_id_t			L$$SELTY = 0;
	acltrl_t			L$$LIT;
	aclspv_wrd_t			L$$NUMWORDS = 1;
	aclspv_wrd_t			L$$NCASE;
	aclspv_wrd_t			L$$NGROUP;
	aclspv_wrd_t			L$$I;
	aclspv_swrd2_t			L$$SELLEN;
	aclspv_bool_t			L$$MERGE_REACHED = 0;
	CXType				L$$SELTYPE;
	h_aclcur_t			L$$P_MERGE;

	(rdwr_body).m_vid = 0;

	L$$COL.m_ctx		= (rdwr_body).m_ctx;
	L$$COL.m_default	= 0;
	L$$COL.m_has_default	= 0;
	L$$COL.m_err		= 0;

	aclmem_init(&L$$COL.m_cases, (rdwr_body).m_ctx);
	aclmem_init(&L$$COL.m_groups, (rdwr_body).m_ctx);

	L$$KINDER.m_cursor	= &L$$CUR[0];
	L$$KINDER.m_num_grepped	= 0;
	L$$KINDER.m_num_wanted	= 2;
	L$$KINDER.m_res		= CXChildVisit_Continue;

	clang_visitChildren(h_cur, s_aclattr_getkinder_visit, &L$$KINDER);

	ae2f_ifnezerr_strict(L$$KINDER.m_num_grepped != 2) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_LACK_COMPONENTS;
		goto L$$LBL_END;
	}

	ae2f_ifnezerr_strict(clang_getCursorKind(L$$CUR[1]) != CXCursor_CompoundStmt) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_LACK_COMPONENTS;
		goto L$$LBL_END;
	}

	L$$SELTYPE = clang_getCursorType(L$$CUR[0]);
	L$$SELLEN = ae2f_static_cast(aclspv_swrd2_t, clang_Type_getSizeOf(L$$SELTYPE));

	ae2f_ifnezerr_strict(L$$SELLEN <= 0 || L$$SELLEN > 8) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_SELECTOR_INVAL;
		goto L$$LBL_END;
	}

	if(L$$SELLEN > 4)
		L$$NUMWORDS = 2;

	switch((aclspv_wrd_t)aclspv_get_expr2(
				&L$$SEL
				, &L$$SELTY
				, L$$CUR[0]
				, &L$$LIT
				, (rdwr_body).m_ctx))
	{
		default:
			(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_SELECTOR_INVAL;
			goto L$$LBL_END;

		case ACLEXPR_RET_NOT_THE_CASE:
		case ACLEXPR_RET_FAILURE:
			(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_SELECTOR_INVAL;
			goto L$$LBL_END;

		case ACLEXPR_RET_SUCCESS:
		case ACLEXPR_RET_SUCCESS_CONSTANT:
			break;

		case ACLEXPR_RET_SUCCESS_LITERAL:
			unless(L$$SEL) {
				ae2f_ifnezerr_strict(
						L$$SELTY < ACLSPV_ID_BOOL
						|| L$$SELTY > ACLSPV_ID_I64) {
					(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_SELECTOR_INVAL;
					goto L$$LBL_END;
				}

				L$$SEL = aclval_cfg_by_enum(
						L$$LIT.m_api_uintmax
						, (rdwr_body).m_ctx
						, (enum ACLSPV_ID_)L$$SELTY);

				ae2f_ifezerr(L$$SEL)
					goto L$$LBL_END;
			}
			break;
	}

	ae2f_ifnezerr((rdwr_body).m_ctx->m_err)
		goto L$$LBL_END;

	L$$MERGE = aclctx_newid((rdwr_body).m_ctx);
	ae2f_ifezerr(L$$MERGE)
		goto L$$LBL_END;

	clang_visitChildren(
			L$$CUR[1]
			, s_aclemit_switch_collect_visit
			, &L$$COL);

	if(L$$COL.m_err) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_SWITCH_CASE_INVAL;
		goto L$$LBL_END;
	}

	ae2f_ifnezerr((rdwr_body).m_ctx->m_err)
		goto L$$LBL_END;

	L$$NCASE	= ae2f_static_cast(aclspv_wrd_t,
			L$$COL.m_cases.m_sz / ae2f_static_cast(aclspv_wrd_t, sizeof(aclcur_sw_entry_t)));
	L$$NGROUP	= ae2f_static_cast(aclspv_wrd_t,
			L$$COL.m_groups.m_sz / ae2f_static_cast(aclspv_wrd_t, sizeof(aclspv_id_t)));
	L$$DEFAULT	= L$$COL.m_has_default ? L$$COL.m_default : L$$MERGE;

	L$$SW = aclcur_cfg_sw(
			L$$SEL
			, L$$DEFAULT
			, L$$MERGE
			, SpvSelectionControlMaskNone
			, L$$NUMWORDS
			, ae2f_reinterpret_cast(const aclcur_sw_entry_t*, L$$COL.m_cases.m_p)
			, L$$NCASE
			, (rdwr_body).m_ctx);

	ae2f_ifnezerr_strict(!L$$SW) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_SWITCH;
		goto L$$LBL_END;
	}

	aclutil_mkname_dbg_fn(
			L$$MERGE
			, "switch"
			, (rdwr_body).m_ctx
			, &(rdwr_body).m_ctx->m_tmp.m_v0);

	L$$EMIT.m_ctx		= (rdwr_body).m_ctx;
	L$$EMIT.m_body		= &(rdwr_body);
	L$$EMIT.m_groups	= ae2f_reinterpret_cast(const aclspv_id_t*, L$$COL.m_groups.m_p);
	L$$EMIT.m_num_groups	= L$$NGROUP;
	L$$EMIT.m_group		= 0;
	L$$EMIT.m_next		= 0;
	L$$EMIT.m_open		= 0;
	L$$EMIT.m_merge		= L$$MERGE;
	L$$EMIT.m_err		= 0;

	L$$BRK_PAR = aclcur_get_break_loop((rdwr_body).m_ctx);
	aclcur_set_break_loop(L$$MERGE, (rdwr_body).m_ctx);

	clang_visitChildren(
			L$$CUR[1]
			, s_aclemit_switch_emit_visit
			, &L$$EMIT);

	if(!L$$EMIT.m_err)
		(void)s_aclemit_switch_close(&L$$EMIT);

	aclcur_set_break_loop(L$$BRK_PAR, (rdwr_body).m_ctx);

	if(L$$EMIT.m_err) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_SWITCH;
		goto L$$LBL_END;
	}

	ae2f_ifnezerr((rdwr_body).m_ctx->m_err)
		goto L$$LBL_END;

	ae2f_ifezerr(aclcur_cfg_lbl(L$$MERGE, (rdwr_body).m_ctx)) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_SWITCH;
		goto L$$LBL_END;
	}

	/** the switch statement always continues here unless every path
	 * returned. the selection-merge edge itself does not count: it is a
	 * structural reference, not a control-flow choice. a default that
	 * falls to the merge and any `break`/fall-through jump do count. */
	if(L$$DEFAULT == L$$MERGE)
		L$$MERGE_REACHED = 1;

	{
		aclspv_wrd_t K;

		for(K = 0; K < (rdwr_body).m_ctx->m_num_cursor; ++K) {
			h_aclcur_t P = aclcur_get_node_idx(K, (rdwr_body).m_ctx);

			if(P && P->m_class == ACLCUR_J && P->m_d.m_j.m_lbl == L$$MERGE) {
				L$$MERGE_REACHED = 1;
				break;
			}
		}
	}

	L$$P_MERGE = aclcur_get_node(L$$MERGE, (rdwr_body).m_ctx);
	unless(L$$P_MERGE) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_SWITCH;
		goto L$$LBL_END;
	}

	aclcur_set_fnret(L$$MERGE_REACHED ? 0 : 1, (rdwr_body).m_ctx);

	/** name the group labels now that they exist. */
	for(L$$I = 0; L$$I < L$$NGROUP; ++L$$I)
		aclutil_mkname_dbg_fn(
				L$$EMIT.m_groups[L$$I]
				, "casebody"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0);

	L$$LBL_END:
		aclmem_fini(&L$$COL.m_groups);
		aclmem_fini(&L$$COL.m_cases);
}

#endif
