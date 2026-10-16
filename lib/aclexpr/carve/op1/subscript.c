#include "./subscript.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/ptr.h>
#include <aclexpr/util/unwrap.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/op.h>
#include <aclcur/reg/view.h>

/** capture the children of a subscript: the array, then the index. */
typedef struct {
	CXCursor	m_c_arr;
	CXCursor	m_c_idx;
	aclspv_bool_t	m_has;
	aclspv_bool_t	m_two;
} s_subscript_opnd_t;

static enum CXChildVisitResult s_subscript_opnd(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	s_subscript_opnd_t* ae2f_restrict h = rdwr_data;

	unless(h->m_has) {
		h->m_c_arr = c_cur;
		h->m_has = 1;
		return CXChildVisit_Continue;
	}

	h->m_c_idx = c_cur;
	h->m_two = 1;
	return CXChildVisit_Break;
}

/** resolve the array register and the index value for a subscript cursor.
 *
 * the index is normally already evaluated and handed in as c_idx_val (the
 * read path cascades both children). for the write path there is no cascade
 * — the lvalue short-circuited — so the index is recovered here, and only
 * when it is a literal or a plain variable: an arbitrary index expression
 * cannot be evaluated from inside a carve, and guessing would silently
 * produce the wrong element.
 *
 * @return non-zero on success.
 * */
static aclspv_bool_t s_subscript_resolve(
		CXCursor			c_sub,
		aclspv_id_t			c_idx_val,
		h_aclspv_ctx_t			h_ctx,
		aclspv_id_t* ae2f_restrict	rdwr_reg,
		aclspv_id_t* ae2f_restrict	rdwr_idx
		) {
	s_subscript_opnd_t O;
	aclspv_id_t REG;
	CXCursor C_IDX;

	O.m_c_arr = clang_getNullCursor();
	O.m_c_idx = clang_getNullCursor();
	O.m_has = 0;
	O.m_two = 0;

	clang_visitChildren(c_sub, s_subscript_opnd, &O);

	if(!O.m_has || !O.m_two)
		return 0;

	ae2f_ifezerr(REG = aclexpr_opnd_reg(aclexpr_expr_unwrap(O.m_c_arr), h_ctx))
		REG = 0;

	if(!REG)
		return 0;

	*rdwr_reg = REG;

	if(c_idx_val) {
		*rdwr_idx = c_idx_val;
		return 1;
	}

	/** no cascade ran: the index must come off the cursor.
	 *
	 * two forms are recoverable here. a plain variable is loaded from its
	 * register; an integer literal is minted as a constant. an arbitrary
	 * index expression cannot be evaluated from inside a carve, and
	 * guessing would silently produce the wrong element — so it is refused.
	 *
	 * a plain load, NOT the explicit one: load.c refuses an explicit load
	 * of a register that already carries a value, which is exactly the
	 * assigned case. plain handles both — tracked registers return their
	 * value, untracked ones take the seed load.
	 *
	 * ptr_taken first: the variable's OpVariable must exist for the index
	 * to be a defined id. */
	C_IDX = aclexpr_expr_unwrap(O.m_c_idx);

	switch((uint_leastenum_t)C_IDX.kind) {
		case CXCursor_IntegerLiteral:
		{
			CXEvalResult EVAL = clang_Cursor_Evaluate(C_IDX);
			aclspv_id_t ID = 0;

			ae2f_ifezerr(EVAL)
				return 0;

			if(clang_EvalResult_getKind(EVAL) == CXEval_Int)
				ID = aclutil_mk_constant_val_id(
						(aclspv_wrd_t)clang_EvalResult_getAsLongLong(EVAL)
						, h_ctx
						);

			clang_EvalResult_dispose(EVAL);

			*rdwr_idx = ID;

			return 0 != ID;
		}

		case CXCursor_DeclRefExpr:
		{
			const aclspv_id_t IREG = aclexpr_opnd_reg(C_IDX, h_ctx);

			if(0 == IREG)
				return 0;

			aclexpr_ptr_taken(IREG, h_ctx);

			ae2f_ifezerr(*rdwr_idx = aclcur_load_reg(IREG, h_ctx, 0))
				*rdwr_idx = 0;

			return 0 != *rdwr_idx;
		}

		default:
			err_puts("[carve1::subscriptwrite] index is neither a literal nor a variable");
			return 0;
	}
}

/** `a[i]` — build the view, then read through it.
 *
 * the array needs no decay: `int a[3]` is already ptr(Function, [3 x int]),
 * so the whole subscript is one OpAccessChain over that pointer, produced by
 * aclcur_regview_addr as a node (so a dynamic index, itself an instruction,
 * is ordered before the chain). an interface-argument register — a pointer
 * to an array, or to the frontend's wrapped-runtime-array struct — resolves
 * the same way: aclexpr_sub_elm_ty (ptr.h) classifies the base into the
 * single-level or the two-level (member-0 select) chain.
 * */
aclexpr_carve_lbl_t aclexpr_carve2_subscript(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_lst_extra,
		ae2f_unused aclutil_literal* ae2f_restrict r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
	unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP2_SUBSCRIPT)
		return c_lbl_continue;

	{
		aclspv_id_t REG;
		aclspv_id_t IDX;
		aclspv_id_t TY_ELM;
		aclspv_id_t TY_MID;
		aclspv_id_t VIEW;
		aclspv_id_t PTR;
		h_aclcur_t P_REG;
		aclspv_wrd_t STORAGE;

		/** the index came from the cascade: slot 1 holds its value id. the
		 * ARRAY however must be reached as a register, not as a value —
		 * aclcur_reg_alias2 links the view to a register node, and slot 0
		 * holds the array's loaded value.
		 *
		 * extra[0].m_cursor is the array's own DeclRefExpr here: the
		 * cascade rewrites that slot per operand, so by now it names the
		 * array, not the subscript. (the write path never cascaded, so it
		 * re-derives from the subscript cursor instead — see below.) */
		ae2f_ifezerr(REG = aclexpr_opnd_reg(
					aclexpr_expr_unwrap(rdwr_lst_extra[0].m_cursor)
					, CTX(rdwr_data)
					))
			REG = 0;

		IDX	= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];

		ae2f_ifnezerr_strict(!REG || !IDX) {
			err_puts("[carve1::subscript] array register or index missing");
			return c_lbl_err;
		}

		ae2f_ifezerr(TY_ELM = aclexpr_sub_elm_ty(
					REG
					, CTX(rdwr_data)
					, &TY_MID
					)) {
			err_puts("[carve1::subscript] the subscripted operand is neither an array nor a pointer");
			return c_lbl_err;
		}

		ae2f_ifezerr(P_REG = aclcur_get_node(REG, CTX(rdwr_data)))
			P_REG = 0;

		if(!P_REG) {
			err_puts("[carve1::subscript] array register missing as node");
			return c_lbl_err;
		}

		/** P_REG is a bare pointer into the register node arena, and
		 * aclcur_reg_view allocates a node into that arena — so the handle
		 * is only good until the view is built. the storage class is the one
		 * field still needed afterwards (to build the element pointer type),
		 * so it is taken here while P_REG is known to be valid. */
		STORAGE = P_REG->m_d.m_reg.m_storage_class;

		/** vector: OpVectorExtractDynamic — no access chain needed.
		 * the vector value is resolved from the register (load for a
		 * memory variable, tracked value for a value register), then the
		 * extract instruction produces the element value directly. */
		if(aclexpr_sub_elm_is_vec(TY_ELM, CTX(rdwr_data))) {
			aclspv_id_t	VEC_VAL;
			aclspv_id_t	ELEM_VAL;

			/** resolve the vector register to its value. */
			ae2f_ifezerr(VEC_VAL = aclexpr_ptrval(REG, CTX(rdwr_data)))
				return c_lbl_err;

			/** `&v[i]`: the address-of wants the element's ADDRESS, not
			 * its value. OpVectorExtractDynamic cannot produce an address
			 * (vectors have no addressability), so we emit the extract and
			 * wrap the result in OpCopyObject to produce a typed address
			 * id — the same pattern the array path uses for &a[i]. */
			if(rdwr_lst_extra[0].m_state0
					& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM) {
				aclspv_id_t	TY_PTR;
				aclspv_id_t	ID_ADDR;

				ae2f_ifezerr(TY_PTR = aclty_mk_ptr(
							TY_ELM
							, (enum SpvStorageClass_)STORAGE
							, CTX(rdwr_data))) {
					err_puts("[carve1::subscript] the element pointer type failed");
					return c_lbl_err;
				}

				ae2f_ifezerr(ELEM_VAL = aclcur_cfg_op2(
							0
							, SpvOpVectorExtractDynamic
							, TY_ELM
							, VEC_VAL
							, IDX
							, CTX(rdwr_data))) {
					err_puts("[carve1::subscript] the vector extract failed");
					return c_lbl_err;
				}

				ae2f_ifezerr(ID_ADDR = aclcur_cfg_op1(
							0
							, SpvOpCopyObject
							, TY_PTR
							, ELEM_VAL
							, CTX(rdwr_data))) {
					err_puts("[carve1::subscript] the element pointer value failed");
					return c_lbl_err;
				}

				*rdwr_prv_idval = ID_ADDR;
				*rdwr_prv_extra = rdwr_lst_extra[0];
				rdwr_prv_extra->m_type_original = TY_PTR;
				rdwr_prv_extra->m_mask_literal = 0;
				rdwr_prv_extra->m_state0
					|= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM;

				return c_lbl_success;
			}

			/** OpVectorExtractDynamic: result type = element type,
			 * operands = vector value + index value. */
			ae2f_ifezerr(ELEM_VAL = aclcur_cfg_op2(
						0
						, SpvOpVectorExtractDynamic
						, TY_ELM
						, VEC_VAL
						, IDX
						, CTX(rdwr_data))) {
				err_puts("[carve1::subscript] the vector extract failed");
				return c_lbl_err;
			}

			*rdwr_prv_idval = ELEM_VAL;
			*rdwr_prv_extra = rdwr_lst_extra[0];
			rdwr_prv_extra->m_type_original = TY_ELM;
			rdwr_prv_extra->m_mask_literal = 0;

			return c_lbl_success;
		}

		/** TY_MID set: the base is a pointer whose pointee is the
		 * wrapped-runtime-array struct, and the chain must descend through
		 * member 0 before the subscript index — two views. every other form
		 * keeps the plain one-view chain. */
		if(TY_MID) {
			aclspv_id_t IDX0;
			aclspv_id_t VIEW_MID;

			/** the member-0 ordinal, minted exactly like a literal
			 * subscript: a cached i32 OpConstant. aclcur_regview_addr
			 * refcounts its chain indices when the address is
			 * materialized, so nothing further is needed here. */
			ae2f_ifezerr(IDX0 = aclutil_mk_constant_val_id(0, CTX(rdwr_data))) {
				err_puts("[carve1::subscript] the member-0 index failed");
				return c_lbl_err;
			}

			/** member-0 select over the base register, then the real
			 * index over that — the chain is (base, 0, idx). the
			 * intermediate view carries member 0's ARRAY type (not the
			 * struct): aclcur_regview_addr types each level's
			 * OpAccessChain as ptr<storage, that level's own m_ty>, and
			 * indexing a struct with the literal 0 must land on the
			 * member's pointer type to validate. */
			ae2f_ifezerr(VIEW_MID = aclcur_reg_view(
						clang_getNullCursor()
						, REG
						, TY_MID
						, IDX0
						, STORAGE
						, ACLCUR_REGVIEW_STEP_ELEMENT
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscript] the member-0 view could not be built");
				return c_lbl_err;
			}

			ae2f_ifezerr(VIEW = aclcur_reg_view(
						rdwr_lst_extra[0].m_cursor
						, VIEW_MID
						, TY_ELM
						, IDX
						, STORAGE
						, ACLCUR_REGVIEW_STEP_ELEMENT
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscript] the view could not be built");
				return c_lbl_err;
			}
		} else {
			ae2f_ifezerr(VIEW = aclcur_reg_view(
						rdwr_lst_extra[0].m_cursor
						, REG
						, TY_ELM
						, IDX
						, STORAGE
						, ACLCUR_REGVIEW_STEP_ELEMENT
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscript] the view could not be built");
				return c_lbl_err;
			}
		}

		/** `&a[i]`: the view is already built and its address taken, and
		 * that address is precisely the value `&` asks for — so hand it on
		 * and emit no OpLoad. the load would be dead code, but worse: it
		 * would consume the same view and leave the address carve with a
		 * plain element value it cannot take the address of. */
		if(rdwr_lst_extra[0].m_state0
				& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM) {
			aclspv_id_t TY_PTR;
			aclspv_id_t ID_ADDR;

			/** the address of the view, minted only on this path: the plain
			 * read below goes through the SSA view load instead, whose emit
			 * hook mints its own chain. one OpAccessChain per chain level,
			 * in cursor order. */
			ae2f_ifezerr(PTR = aclcur_regview_addr(VIEW, CTX(rdwr_data))) {
				err_puts("[carve1::subscript] the view address failed");
				return c_lbl_err;
			}

			ae2f_ifezerr(TY_PTR = aclty_mk_ptr(
						TY_ELM
						, (enum SpvStorageClass_)STORAGE
						, CTX(rdwr_data))) {
				err_puts("[carve1::subscript] the element pointer type failed");
				return c_lbl_err;
			}

			/** PTR is an address, not a value: handing the id on as-is
			 * leaves the store into `int* p2` type-checking a bare address
			 * with no VALUE flag, so OpCopyObject re-types it.
			 *
			 * the REFCOUNTED helper, not aclcur_cfg_op1r: op1r's operand is
			 * "emitted verbatim and neither validated nor refcounted", which
			 * is right for the plain `&a` case (there the operand is a
			 * register id, and a register is never dropped). here the
			 * operand is the access-chain NODE, and the emit pass elides any
			 * ACLCUR_OP1 with m_num_ref == 0 — op1r would leave the chain
			 * unreferenced, the elision would swallow the OpAccessChain, and
			 * the module would name an id nothing defines. */
			ae2f_ifezerr(ID_ADDR = aclcur_cfg_op1(
						0
						, SpvOpCopyObject
						, TY_PTR
						, PTR
						, CTX(rdwr_data))) {
				err_puts("[carve1::subscript] the element pointer value failed");
				return c_lbl_err;
			}

			*rdwr_prv_idval = ID_ADDR;
			*rdwr_prv_extra = rdwr_lst_extra[0];
			rdwr_prv_extra->m_type_original = TY_PTR;
			/** an address is never a compile-time literal. */
			rdwr_prv_extra->m_mask_literal = 0;
			/** re-set on the way back up: by now the cascade has rewritten
			 * extra[0].m_cursor to name the ARRAY, so the cursor alone can
			 * no longer tell `&a[i]` from `&a` — this flag can. */
			rdwr_prv_extra->m_state0
				|= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM;

			return c_lbl_success;
		}

		/** the value of the view, read as an SSA l-value: aclcur_load_reg
		 * resolves the block-local value / phi and falls back to a seed
		 * OpLoad through the view's own chain, which the view hook mints
		 * as raw words. */
		ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(
					VIEW
					, CTX(rdwr_data)
					, 0
					)) {
			err_puts("[carve1::subscript] the load failed");
			return c_lbl_err;
		}

		*rdwr_prv_extra = rdwr_lst_extra[0];
		rdwr_prv_extra->m_type_original = TY_ELM;
		rdwr_prv_extra->m_mask_literal = 0;
	}

	return c_lbl_success;
}

/** `a[i] = v` — build the view, then store through it.
 *
 * the RHS arrives in operand slot 1; the array and index are re-derived from
 * the lvalue cursor because the read cascade was suppressed for this operand.
 * */
aclexpr_carve_lbl_t aclexpr_carve2_subscriptwrite(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_lst_extra,
		aclutil_literal* ae2f_restrict		r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
		unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP2_SUBSCRIPTWRITE)
		return c_lbl_continue;

	{
		const CXCursor SUB = rdwr_lst_extra[0].m_cursor;
		aclspv_id_t REG;
		aclspv_id_t IDX;
		aclspv_id_t TY_ELM;
		aclspv_id_t TY_MID;
		aclspv_id_t VIEW;
		aclspv_wrd_t STORAGE;
		h_aclcur_t P_REG;
		const aclspv_id_t X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];

		ae2f_ifnezerr_strict(!s_subscript_resolve(
					SUB
					, 0
					, CTX(rdwr_data)
					, &REG
					, &IDX
					)) {
			err_puts("[carve1::subscriptwrite] could not resolve array and index");
			return c_lbl_err;
		}

		ae2f_ifnezerr_strict(!X) {
			err_puts("[carve1::subscriptwrite] no value to store");
			return c_lbl_err;
		}

		ae2f_ifezerr(TY_ELM = aclexpr_sub_elm_ty(
					REG
					, CTX(rdwr_data)
					, &TY_MID
					)) {
			err_puts("[carve1::subscriptwrite] the subscripted operand is neither an array nor a pointer");
			return c_lbl_err;
		}

		ae2f_ifezerr(P_REG = aclcur_get_node(REG, CTX(rdwr_data)))
			P_REG = 0;

		if(!P_REG) {
			err_puts("[carve1::subscriptwrite] array register missing as node");
			return c_lbl_err;
		}

		/** captured before any view is built: the node handle is a bare
		 * arena pointer that aclcur_reg_view (by allocating into the same
		 * arena) can invalidate, and the constant mint below runs first on
		 * the two-level path. */
		STORAGE = P_REG->m_d.m_reg.m_storage_class;

		/** vector: OpVectorInsertDynamic — produce a new vector value
		 * with the element inserted at the index, then store it back to
		 * the register. mirrors the array path (view + aclcur_store_reg)
		 * but uses insert + direct register store since vectors are
		 * values, not memory objects addressed by an access chain. */
		if(aclexpr_sub_elm_is_vec(TY_ELM, CTX(rdwr_data))) {
			aclspv_id_t	VEC_VAL;
			aclspv_id_t	NEW_VEC;

			/** resolve the vector register to its value. */
			ae2f_ifezerr(VEC_VAL = aclexpr_ptrval(REG, CTX(rdwr_data)))
				return c_lbl_err;

			/** OpVectorInsertDynamic: result = new vector, operands =
			 * vector, element value, index (literal). */
			ae2f_ifezerr(NEW_VEC = aclcur_cfg_op2i(
						0
						, SpvOpVectorInsertDynamic
						, REG
						, VEC_VAL
						, X
						, (aclspv_wrd_t)IDX
						, CTX(rdwr_data))) {
				err_puts("[carve1::subscriptwrite] the vector insert failed");
				return c_lbl_err;
			}

			/** store the new vector back to the register:
			 * aclcur_store_reg handles the register-level assignment
			 * (phi for scoped, explicit store for non-scoped). */
			ae2f_ifezerr(aclcur_store_reg(
						REG
						, NEW_VEC
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscriptwrite] vector store failed");
				return c_lbl_err;
			}

			*rdwr_prv_idval = NEW_VEC;
			*rdwr_prv_extra = rdwr_lst_extra[1];
			rdwr_prv_extra->m_type_original = REG;
			rdwr_prv_extra->m_mask_literal = 0;
			rdwr_prv_extra->m_literal = *r_ltrl;

			return c_lbl_success;
		}

		/** the same two-level chain as the read path when the base is a
		 * pointer to the wrapped-runtime-array struct. */
		if(TY_MID) {
			aclspv_id_t IDX0;
			aclspv_id_t VIEW_MID;

			ae2f_ifezerr(IDX0 = aclutil_mk_constant_val_id(0, CTX(rdwr_data))) {
				err_puts("[carve1::subscriptwrite] the member-0 index failed");
				return c_lbl_err;
			}

			ae2f_ifezerr(VIEW_MID = aclcur_reg_view(
						clang_getNullCursor()
						, REG
						, TY_MID
						, IDX0
						, STORAGE
						, ACLCUR_REGVIEW_STEP_ELEMENT
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscriptwrite] the member-0 view could not be built");
				return c_lbl_err;
			}

			ae2f_ifezerr(VIEW = aclcur_reg_view(
						SUB
						, VIEW_MID
						, TY_ELM
						, IDX
						, STORAGE
						, ACLCUR_REGVIEW_STEP_ELEMENT
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscriptwrite] the view could not be built");
				return c_lbl_err;
			}
		} else {
			ae2f_ifezerr(VIEW = aclcur_reg_view(
						SUB
						, REG
						, TY_ELM
						, IDX
						, STORAGE
						, ACLCUR_REGVIEW_STEP_ELEMENT
						, CTX(rdwr_data)
						)) {
				err_puts("[carve1::subscriptwrite] the view could not be built");
				return c_lbl_err;
			}
		}

		/** store through the view: aclcur_store_reg validates the SSA-scope
		 * policy (ACLCUR_FLAG0_REG_SCOPED) uniformly — a non-scoped view's
		 * store is explicit and reaches memory, through a chain the view's
		 * own emit hook mints. no PTR is needed here. */
		ae2f_ifezerr(aclcur_store_reg(
					VIEW
					, X
					, CTX(rdwr_data)
					)) {
			err_puts("[carve1::subscriptwrite] the store failed");
			return c_lbl_err;
		}

		*rdwr_prv_idval = X;
	}

	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return c_lbl_success;
}
