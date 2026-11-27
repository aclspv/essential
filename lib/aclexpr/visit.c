/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

/** @file visit.c */

#include <aclutil/scale.h>

#include <aclmem.h>
#include <aclexpr/op.h>

#include <aclctx/id.h>
#include <aclctx/err.h>

#include "./carve.h"

#include "./visit/api.h"
#include "./visit/val.h"
#include "./visit/op1/arithmetic_cast.h"
#include "./visit/op1/deref.h"
#include "./visit/op1/incr.h"
#include "./visit/op1/memberread.h"
#include "./visit/op1/subscript.h"
#include "./visit/op1/neg.h"
#include "./visit/op1/plus.h"
#include "./visit/op1/not.h"
#include "./visit/opn/initlist.h"
#include "./visit/op2/arithmetic.h"
#include "./visit/op2/assign.h"
#include "./visit/op2/bool0.h"
#include "./visit/op2/cmp.h"
#include "./visit/op2/shift.h"
#include "./visit/op2/comma.h"
#include "./visit/op3/cond.h"
#include "./visit/op1/glslmath.h"
#include "./visit/opn/shuffle.h"
#include "./visit/opn/call.h"

#include "./beforevisit.h"

#include <aclutil/emitx.h>
#include <aclutil/fn.h>
#include <aclexpr/util/unwrap.h>

enum OP_ {
	OP2_ARITHMETIC,
	OP2_CMP,
	OP2_SHIFT,
	OP2_BOOL0,
	OP2_COMMA,
	OP2_ASSIGN,
	OP1_ARITHMETIC_CAST,
	OP1_INCR,
	OP1_SUBSCRIPT,
	OP1_MEMBERREAD,
	OP1_DEREF,
	OP1_NEG,
	OP1_PLUS,
	OP1_NOT,
	OP1_BITNOT,
	OP3_COND,
	OP1_GLSL_MATH,
	OPN_CALL,
	OPN_SHUFFLE,
	OPN_INITLIST,
	OP0_,
	OP0_VAL,
	OP_DONE,
	OP_FAIL,
	OP_FALL,
	OP_RECURSE,
	OP_RECURSE_NOTHIN,
	OP1_NEG_SUCCESS,
	OP_STMTEXPR,
	OP_ENTRY_POINT
};

enum CXChildVisitResult aclexpr_visit(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData rdwr_data
		)
{
#define	BASE			((struct aclexpr_visit_data* ae2f_restrict)(rdwr_data))
#define	CTX			BASE->m_ctx
#define ID_REQ			BASE->m_id_ret 
#define	IS_NOT_CONSTANT		BASE->m_literal_flag 
#define	TYPE_CAST_CURRENT	BASE->m_tyid_current 
#define	NUM_CMDCOUNTERSTCK	BASE->m_num_cmdcounter
#define	LITERAL			BASE->m_ret_literal

#define	CMDSTCK_SCALE	CTX->m_body.m_bootscale
#define	CMDCOUNTERSTCK	CTX->m_body.m_cmdcountstck

	ae2f_unused typedef char s_ASSRT_ELMSIZE[
		sizeof(aclexprstck) == sizeof(aclexpr_cmdcounterstckelm) && sizeof(aclexpr_cmdcounterstckelm) == sizeof(u32_least)
	];

#define	BUF_CMDCOUNTERSTCK(a)	((aclexpr_cmdcounterstckelm* ae2f_restrict)(CMDCOUNTERSTCK).m_p)
#define	GETLASTEL_CMDCOUNTERSTCK(a)	BUF_CMDCOUNTERSTCK()[NUM_CMDCOUNTERSTCK - 1]
#define	GETLASTEL_CMDCOUNTERSTCK_PRV(a)	BUF_CMDCOUNTERSTCK()[NUM_CMDCOUNTERSTCK - 2]

	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE);
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
	acltrl_t* ae2f_restrict RET_LITERAL		= &LITERAL;

	aclexpr_opbincfg_extra* ae2f_restrict	LST_EXTRA;

	dbg_prefix(i);
	dbg_call(fprintf, (stderr, "[visit] OPR: %u/%u ID(scale): %u\n"
					, (unsigned)GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd
					, (unsigned)LST_SCALE_BUF[0]
					, (unsigned)LST_SCALE->m_id
				    ));

	aclctx_err_check(CTX) {
		err_prefix();
		err_call(fprintf, (stderr, "[visit] error %u\n"
						, (unsigned)CTX->m_err
					    ));
		return CXChildVisit_Break;
	}

	
	ae2f_ifezerr(NUM_CMDCOUNTERSTCK) {
		_aclctx_err(CTX, ACLSPV_COMPILE_TOO_BIG_EXPRVISIT);
		return CXChildVisit_Break;
	}

	ae2f_expected_but_else(LST_SCALE_BUF) {
		_aclctx_err(CTX, ACLSPV_COMPILE_ALLOC_FAILED_EXPR);
		return CXChildVisit_Break;
	}

	LST_EXTRA = (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[0])]);
	LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal = ACLTRL_MASK_LTRL;

	/** a copy/move constructor with no user-provided body is a plain value
	 * copy. a user-provided one is dispatched to the call lane below. */
	if(aclutil_cursor_is_copy_move_ctor_call(h_cur)) {
		CXCursor CTOR = clang_getCursorReferenced(h_cur);

		unless(aclutil_ctor_has_body(CTOR)) {
			CXCursor ARG = clang_Cursor_getArgument(h_cur, 0);

			if(clang_Cursor_isNull(ARG))
				return CXChildVisit_Break;

			return aclutil_visitfromroot(ARG, aclexpr_visit, rdwr_data);
		}
	}

	/** std::move is a transparent cast: evaluate its operand, but reserve
	 * the moved-from object as no longer owning a destructor. */
	if(aclutil_cursor_is_std_move(h_cur)) {
		CXCursor ARG = clang_Cursor_getArgument(h_cur, 0);
		CXCursor SRC = aclexpr_expr_unwrap(ARG);
		enum CXChildVisitResult R;

		/** evaluate first: member/subscript l-values materialise their view
		 * registers during this walk. */
		R = aclutil_visitfromroot(ARG, aclexpr_visit, rdwr_data);

		switch((aclspv_cxcurkind_t)clang_getCursorKind(SRC)) {
			default:
				break;

			case CXCursor_DeclRefExpr:
			case CXCursor_MemberRefExpr:
			case CXCursor_ArraySubscriptExpr:
				{
					aclspv_id_t REG = aclexpr_curreg_find(SRC, CTX);
					h_aclcur_t p_reg;

					if(!REG)
						REG = aclexpr_curreg_find(
								clang_getCursorReferenced(SRC), CTX);

					p_reg = aclcur_get_node(REG, CTX);

					if(p_reg)
						p_reg->m_flag0 &=
							(aclspv_wrd_t)~aclcur_flag0_reg(
								ACLCUR_FLAG0_REG_DTOR);
				}
				break;
		}

		return R;
	}

	/** Recurse filter */
	switch((uintmax_t)h_cur.kind) {
		default:
			break;
		case CXCursor_UnexposedExpr:
			/** libclang surfaces `__builtin_shufflevector` as an
			 * UnexposedExpr, but it is a real operator and must reach the
			 * dispatch chain. every other UnexposedExpr is a transparent
			 * wrapper. */
			unless(aclexpr_shuffle_detect(h_cur))
				return CXChildVisit_Recurse;
			break;
	}

	LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_cursor = h_cur;

#if	aclspv_cgoto
#	define	cgswtch(a)		goto LBL_##a;
#	define	cgcsdef(a)		LBL_##a
#	define	cgdefault		cgcsdef
#	define	cgcase(a)		(&&LBL_##a)
#	define	cgjmp(a)		ae2f_cgoto_beg(); goto *(a); ae2f_cgoto_end()
#else
#	define	cgswtch(a)		enum OP_ LABELJMP = a; LABELJMPINIT: switch(LABELJMP)
#	define	cgcsdef(a)		case a
#	define	cgdefault(a)		case a: default
#	define	cgcase(a)		a
#	define	cgjmp(a)		LABELJMP = a; goto LABELJMPINIT
#endif

	dbg_puts("[visit] CGSWTCH");
	cgswtch(OP_ENTRY_POINT) {
		cgcsdef(OP_FAIL):
		{
			err_puts("[visit] OP_FAIL");
			return CXChildVisit_Break;
		}

		cgcsdef(OP_RECURSE):
		{
			ae2f_ifnezerr_strict(s_aclmem_cgrow2(
					&CMDCOUNTERSTCK
					, NUM_CMDCOUNTERSTCK + 1
					, sizeof(aclexpr_cmdcounterstckelm)
					, ACLSPV_COMPILE_ALLOC_FAILED_EXPR
				       )) return CXChildVisit_Break;

			(void)++NUM_CMDCOUNTERSTCK;
			GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd = 0;
			return CXChildVisit_Recurse;
		}

		cgcsdef(OP_RECURSE_NOTHIN):
		{
			return CXChildVisit_Recurse;
		}

		cgcsdef(OP0_):
		{
			dbg_puts("[visit] OP0_");

			/** a lane may create a child scale and return here directly
			 * (implicit-this member read); re-sync to the last scale. */
			LST_SCALE = get_last_scale_from_vec(&CMDSTCK_SCALE);
			LST_SCALE_BUF = get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
			LST_EXTRA = (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND])]);

			if(LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal & (aclspv_wrd_t)ACLTRL_LTRL) {
				*RET_LITERAL = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(i).m_count_oprnd].m_literal; 
			}

			IS_NOT_CONSTANT &= LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_mask_literal;

			LST_SCALE_BUF[ACLEXPR_OPBINCFG_END + GETLASTEL_CMDCOUNTERSTCK(i).m_count_oprnd] = ID_REQ;
			TYPE_CAST_CURRENT = LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd].m_type_original;

			(void)(LST_SCALE->m_id && ++GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd);

			while(LST_SCALE->m_id && GETLASTEL_CMDCOUNTERSTCK().m_count_oprnd == LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND]) {
				aclspv_wrd_t	IDVAL_PRV = 0;
				aclexpr_opbincfg_extra PRV_EXTRA;

				aclmemzero(&PRV_EXTRA, sizeof(PRV_EXTRA));

				LST_SCALE		= get_last_scale_from_vec(&CMDSTCK_SCALE);
				LST_SCALE_BUF		= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);

				ae2fpat_assertassume(LST_SCALE_BUF);

				LST_EXTRA
					= (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND])]);

				ae2f_ifnezerr_strict(NUM_CMDCOUNTERSTCK < 2) {
					err_puts("[visit] NUM_CMDCOUNTERSTCK is fishy");
					return CXChildVisit_Break;
				}

				dbg_prefix();
				dbg_call((void), fprintf(stderr
							, "[visit] carve: depth %u prvidx %u (before)\n"
							, (unsigned)NUM_CMDCOUNTERSTCK - 1u
							, (unsigned)GETLASTEL_CMDCOUNTERSTCK_PRV(a).m_count_oprnd));

				ae2f_ifnezerr_strict(aclexpr_carve(
							rdwr_data
							, &IDVAL_PRV
							, &PRV_EXTRA))
					return CXChildVisit_Break;

				ae2f_ifezerr(IDVAL_PRV) {
					return CXChildVisit_Break;
				}

				del_scale_from_vec_last(&CMDSTCK_SCALE);

				ae2f_ifezerr(NUM_CMDCOUNTERSTCK) {
					err_puts("[visit] command counter stack has been broken.");
					return CXChildVisit_Break;
				} 

				(void)--NUM_CMDCOUNTERSTCK;

				dbg_prefix();
				dbg_call((void), fprintf(stderr
							, "[visit] carve: depth %u prvidx %u (after)\n"
							, (unsigned)NUM_CMDCOUNTERSTCK - 1u
							, (unsigned)GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd));

				LST_SCALE		= get_last_scale_from_vec(&CMDSTCK_SCALE);
				ae2f_expected_but_else(LST_SCALE) return CXChildVisit_Break;
				LST_SCALE_BUF		= get_buf_from_scale2(aclspv_wrd_t, &CMDSTCK_SCALE, *LST_SCALE);
				ae2f_expected_but_else(LST_SCALE_BUF) return CXChildVisit_Break;
				LST_EXTRA		= (void* ae2f_restrict)(&LST_SCALE_BUF[(ACLEXPR_OPBINCFG_END + LST_SCALE_BUF[0])]);

				/** NOPURE is OR-propagated: any impure operand makes the
				 * result impure. pure is the default clear state. */
				{
					aclspv_wrd_t NOPURE = PRV_EXTRA.m_state0 & (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_NOPURE;
					aclspv_wrd_t i;

					for(i = 0; i < LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND]; ++i)
						NOPURE |= LST_EXTRA[i].m_state0 & (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_NOPURE;

					PRV_EXTRA.m_state0 = (PRV_EXTRA.m_state0 & ~(aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_NOPURE) | NOPURE;
				}

				LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd] = PRV_EXTRA;

				ID_REQ = LST_SCALE_BUF[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd + ACLEXPR_OPBINCFG_END] = IDVAL_PRV;
				BASE->m_tyid_current = PRV_EXTRA.m_type_original;
				BASE->m_literal_flag	&= PRV_EXTRA.m_mask_literal;
				BASE->m_ret_literal	= PRV_EXTRA.m_literal;

				(void)(LST_SCALE->m_id && ++GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd);
			}

			return ae2f_expected_not(CTX->m_err) ? CXChildVisit_Break : CXChildVisit_Continue;
		}

		cgcsdef(OP_ENTRY_POINT):
		{
			/** a statement expression is a value leaf: its statements are
			 * carved as a block and the block's last value becomes the
			 * operand. it must not recurse, or its statements would be
			 * parsed as operands of the enclosing operator. */
			if((aclspv_cxcurkind_t)h_cur.kind == CXCursor_StmtExpr) {
				cgjmp(cgcase(OP_STMTEXPR));
			}

			cgjmp(
					aclexpr_beforevisit(h_cur, rdwr_data, GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd)
					? cgcase(OP_FAIL)
					: cgcase(OP1_GLSL_MATH)
			     );
		}

		cgcsdef(OP_STMTEXPR):
		{
			aclexpr_opbincfg_extra* ae2f_restrict EXTRA
				= &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd];
			aclspv_id_t	IDVAL;
			aclty_id_t	TYID;

			unless(CTX->m_stmtexpr) {
				err_puts("[visit] statement expression outside body emission");
				return CXChildVisit_Break;
			}

			IDVAL = CTX->m_stmtexpr(h_cur, &TYID, CTX);

			aclctx_err_check(CTX)
				return CXChildVisit_Break;

			ae2f_ifezerr(IDVAL) {
				dbg_puts("[visit] statement expression yielded no value");
				return CXChildVisit_Break;
			}

			/** the type came from the carved value, not from the cursor. */
			ae2f_ifezerr(TYID) {
				dbg_puts("[visit] statement expression yielded no type");
				return CXChildVisit_Break;
			}

			/** the block's value is a runtime operand. constant folding of
			 * a statement expression's tail is not attempted: the tail may
			 * have been observed through control flow. */
			EXTRA->m_mask_literal	= 0;
			EXTRA->m_type_original	= TYID;

			ID_REQ = IDVAL;

			cgjmp(cgcase(OP0_));
		}

		cgcsdef(OP1_GLSL_MATH):
		{
			cgjmp(aclexpr_visit_op1_glsl_math(
						h_cur, rdwr_data
						, cgcase(OP_DONE)
						, cgcase(OP_FAIL)
						, cgcase(OPN_CALL)
						));
		}

		cgcsdef(OPN_CALL):
		{
			cgjmp(aclexpr_visit_opn_call(
						h_cur, rdwr_data
						, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd]
						, cgcase(OP_DONE)
						, cgcase(OP0_)
						, cgcase(OP_FAIL)
						, cgcase(OPN_SHUFFLE)
						));
		}

		cgcsdef(OPN_SHUFFLE):
		{
			cgjmp(aclexpr_visit_opn_shuffle(
						h_cur, rdwr_data
						, cgcase(OP_DONE)
						, cgcase(OP_FAIL)
						, cgcase(OP3_COND)
						));
		}

		cgcsdef(OP3_COND):
		{
			cgjmp(aclexpr_visit_op3_cond(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OPN_INITLIST)
						));
		}

		cgcsdef(OP_DONE):
		{
			return CXChildVisit_Continue;
		}

		cgcsdef(OPN_INITLIST):
		{
			cgjmp(aclexpr_visit_opn_initlist(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_ARITHMETIC)
						));
		}

		cgcsdef(OP2_ARITHMETIC): 
		{
			cgjmp(aclexpr_visit_op2_arithmetic(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_CMP)
						));
		}

		cgcsdef(OP2_CMP):
		{
			cgjmp(aclexpr_visit_op2_cmp(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_SHIFT)
						));
		}

		cgcsdef(OP2_SHIFT):
		{
			cgjmp(aclexpr_visit_op2_shift(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_BOOL0)
						));
		}

		cgcsdef(OP2_BOOL0):
		{
			cgjmp(aclexpr_visit_op2_bool0(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_COMMA)
						));
		}

		cgcsdef(OP2_COMMA):
		{
			cgjmp(aclexpr_visit_op2_comma(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP2_ASSIGN)
						));
		}

		cgcsdef(OP2_ASSIGN):
		{
			cgjmp(aclexpr_visit_op2_assign(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_ARITHMETIC_CAST)
						));
		}

		cgcsdef(OP1_ARITHMETIC_CAST):
		{
			cgjmp(aclexpr_visit_op1_arithmetic_cast(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_NEG)
						));
		}

		cgcsdef(OP1_NEG):
		{
			cgjmp(aclexpr_visit_op1_neg(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP1_NEG_SUCCESS)
						, cgcase(OP_FAIL)
						, cgcase(OP1_PLUS)
						));
		}

		cgcsdef(OP1_NEG_SUCCESS):
		{
			ae2f_ifnezerr_strict(s_aclmem_cgrow2(
						&CMDCOUNTERSTCK
						, NUM_CMDCOUNTERSTCK + 1
						, sizeof(aclexpr_cmdcounterstckelm)
						, ACLSPV_COMPILE_ALLOC_FAILED_EXPR
						)) return CXChildVisit_Break;

			(void)++NUM_CMDCOUNTERSTCK;
			GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd = 1;
			return CXChildVisit_Recurse;
		}

		cgcsdef(OP1_PLUS):
		{
			cgjmp(aclexpr_visit_op1_plus(h_cur
						, cgcase(OP_RECURSE_NOTHIN)
						, cgcase(OP1_NOT)));
		}

		cgcsdef(OP1_NOT):
		{
			cgjmp(aclexpr_visit_op1_not(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_BITNOT)
						));
		}

		cgcsdef(OP1_BITNOT):
		{
			cgjmp(aclexpr_visit_op1_bitnot(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_INCR)
						));
		}

		cgcsdef(OP1_INCR):
		{
			cgjmp(aclexpr_visit_op1_incr(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_SUBSCRIPT)
						));
		}

		cgcsdef(OP1_SUBSCRIPT):
		{
			cgjmp(aclexpr_visit_op1_subscript(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd]
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_DEREF)
						, cgcase(OP0_)
						));
		}

		cgcsdef(OP1_DEREF):
		{
			cgjmp(aclexpr_visit_op1_deref(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd]
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP1_MEMBERREAD)
						, cgcase(OP0_)
						));
		}

		cgcsdef(OP1_MEMBERREAD):
		{
			cgjmp(aclexpr_visit_op1_memberread(
						h_cur, CTX
						, &CMDSTCK_SCALE
						, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd]
						, &NUM_CMDCOUNTERSTCK
						, &ID_REQ
						, cgcase(OP_RECURSE)
						, cgcase(OP_FAIL)
						, cgcase(OP0_VAL)
						, cgcase(OP0_)
						));
		}

		cgcsdef(OP0_VAL):
		{
			cgjmp(aclexpr_visit_op0_val(
						h_cur, BASE->m_ctx
						, &LST_EXTRA[GETLASTEL_CMDCOUNTERSTCK(a).m_count_oprnd]
						, &ID_REQ
						, cgcase(OP0_)
						, cgcase(OP_FAIL)
						, cgcase(OP_FALL)
						));
		}

		cgdefault(OP_FALL):
		{
			CXString SPELL = clang_getCursorSpelling(h_cur);
			CXString KINDSPELL = clang_getCursorKindSpelling(h_cur.kind);
			aclspv_bool_t IS_OPERATOR = 0;

			dbg_puts("[visit] OP_FALL");

			switch((uintmax_t)h_cur.kind) {
				default:
					break;

				/** operator-shaped expressions are claimed by a lane
				 * above. reaching here means this compiler has no lane
				 * for it: recursing would silently drop the operator
				 * (`!x` compiled as `x`), so reject it instead. */
				case CXCursor_UnaryOperator:
				case CXCursor_UnaryExpr:
				case CXCursor_BinaryOperator:
				case CXCursor_CompoundAssignOperator:
				case CXCursor_ConditionalOperator:
				case CXCursor_ArraySubscriptExpr:
				case CXCursor_MemberRefExpr:
				case CXCursor_CallExpr:
				case CXCursor_CStyleCastExpr:
				case CXCursor_CXXStaticCastExpr:
				case CXCursor_CXXDynamicCastExpr:
				case CXCursor_CXXReinterpretCastExpr:
				case CXCursor_CXXConstCastExpr:
				case CXCursor_CXXFunctionalCastExpr:
					IS_OPERATOR = 1;
					break;
			}

			if(IS_OPERATOR) {
				err_prefix(i);
				err_call(fprintf, (stderr
							, "%s"
							, (const char*)KINDSPELL.data));

				clang_disposeString(SPELL);
				clang_disposeString(KINDSPELL);

				/** a user-facing reject of a construct this emitter
				 * does not model. */

				return CXChildVisit_Break;
			}

			warn_prefix(i);
			warn_call(fprintf, (stderr, "[visit] OP_FALL::spell(%s)::kind(%s)\n"
						, (const char*)SPELL.data
						, (const char*)KINDSPELL.data));

			clang_disposeString(SPELL);
			clang_disposeString(KINDSPELL);

			return CXChildVisit_Recurse;
		}
	}

	ae2f_unreachable();

#if	WARN



#endif

#undef	CTX
#undef	CMDSTCK_SCALE
#undef	ID_REQ
#undef	LST_CMD_COUNT
#undef	LST_CMDBUF_COUNT
#undef	CMDCOUNTERSTCK
#undef	NUM_CMDCOUNTERSTCK
#undef	GETLASTEL_CMDCOUNTERSTCK
#undef	BUF_CMDCOUNTERSTCK
}
