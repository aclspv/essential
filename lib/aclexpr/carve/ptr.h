#ifndef	aclexpr_carve_ptr_h
#define	aclexpr_carve_ptr_h

#include <aclspv.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/load.h>
#include <aclexpr/curreg.h>

#include <aclutil/capability.h>

/** first child cursor of an expression node: the operand of `x` in `&x`,
 * the operand of `q` in `*q`. */
typedef struct {
	CXCursor	m_c;
	aclspv_bool_t	m_has;
} aclexpr_opnd_t;

ae2f_inline_f static enum CXChildVisitResult aclexpr_opnd_visit(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	aclexpr_opnd_t* ae2f_restrict h = rdwr_data;

	h->m_c = c_cur;
	h->m_has = 1;
	return CXChildVisit_Break;
}

/** first child cursor of an expression node.
 *
 * needed only where the visitor did NOT recurse into the operand: an
 * assignment lvalue short-circuits on STATE0_VAL_NO_NEED before the child
 * cascade, so there extra[0].m_cursor is still the operator itself.
 * */
ae2f_inline_f static aclexpr_opnd_t aclexpr_opnd_first_child(
		CXCursor	c_op
		) {
	aclexpr_opnd_t O;

	O.m_c = clang_getNullCursor();
	O.m_has = 0;

	clang_visitChildren(c_op, aclexpr_opnd_visit, &O);

	return O;
}

/** the register bound to an operand cursor.
 *
 * the cursor is re-derived rather than read off the collected operand,
 * because the collected operand is the operand's VALUE (a load node for a
 * memory variable) and a value carries no address — the same move the
 * member-write carve makes for its base.
 *
 * note which cursor that is: on a read the visitor recursed into the
 * operand, and visit.c overwrites extra[0].m_cursor with each cursor on the
 * way down, so extra[0] is already the operand. on an assignment lvalue it
 * short-circuited before recursing, so extra[0] is still the operator and
 * its first child must be taken first.
 *
 * @return 0 when no register is bound to that cursor.
 * */
ae2f_inline_f static aclspv_id_t aclexpr_opnd_reg(
		CXCursor	c_opnd,
		h_aclspv_ctx_t	h_ctx
		) {
	return aclexpr_curreg_find(clang_getCursorReferenced(c_opnd), h_ctx);
}

/** the storage of a register whose ADDRESS is taken must exist.
 *
 * the keep scan materialises a variable's OpVariable only when the register
 * is live (a load or an explicit store) or ACLCUR_FLAG0_REG_ALLOCATED.
 * taking its address is neither: without this the OpVariable is dropped and
 * the address ends up referencing an id nothing defines.
 * */
ae2f_inline_f static void aclexpr_ptr_taken(
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);

	if(p_reg && p_reg->m_class == ACLCUR_REG)
		p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALLOCATED);
}

/** the element type of an ARRAY-typed register, 0 when it is not an array.
 *
 * an array needs no decay to be subscripted: `int a[3]` is already
 * ptr(Function, [3 x int]), so `a[i]` is one OpAccessChain from that pointer.
 * */
ae2f_inline_f static aclspv_id_t aclexpr_arr_elm_ty(
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	const h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);
	const aclty_t* const ae2f_restrict P_TY
		= p_reg ? aclty_find(p_reg->m_ty, h_ctx) : 0;

	if(!P_TY || P_TY->m_class != ACLTY_ARR)
		return 0;

	return P_TY->m_info.m_arr.m_type;
}

/** the SUBSCRIPT lvalue shape of one base register: the LEAF element type
 * and — for the frontend's wrapped-runtime-array idiom — the intermediate
 * view's type.
 *
 * a register feeding `a[i]` is either
 * - an ARRAY (m_ty class ACLTY_ARR): the existing single-level form, whose
 *   element type is m_info.m_arr.m_type;
 * - a POINTER (m_ty class ACLTY_PTR, i.e. an entry-point interface
 *   argument): the pointee type decides —
 *   . pointee ARRAY: single level, element = the pointee's element
 *     (`local float x[N]`, Input/Output `float x[]`-params, any
 *     pointer-to-array);
 *   . pointee STRUCT whose member 0 is an ARRAY (the wrapped-runtime-array
 *     idiom `struct { T[] }`): TWO levels — a literal-0 member select over
 *     the base, then the subscript index;
 *   . anything else (a plain value, or a struct without an array member 0):
 *     single level, element = the pointee itself. indexing past member 0 of
 *     a single-element wrapped struct is a pre-existing frontend
 *     limitation, not worsened here.
 *
 * @param[out] rdwr_ty_mid the intermediate view's m_ty when the chain is
 * two-level, 0 for the single-level forms.
 *
 * the intermediate type is member 0's ARRAY type, NOT the struct: the
 * chain is base-/0const-/idx, and aclcur_regview_addr types each level's
 * OpAccessChain as aclty_mk_ptr(that level's own m_ty, storage). the
 * 0const step lands on member 0, so its pointer type must be
 * ptr<storage, member-0 array> — an OpAccessChain over a struct that is
 * typed by the struct itself does not validate.
 *
 * @return the LEAF view's m_ty, 0 when the register is neither an array
 * nor a pointer (the subscript then has nothing it can index).
 * */
ae2f_inline_f static aclspv_id_t aclexpr_sub_elm_ty(
		aclspv_id_t			c_reg,
		h_aclspv_ctx_t			h_ctx,
		aclspv_id_t* ae2f_restrict	rdwr_ty_mid
		) {
	const h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);
	const aclty_t* const ae2f_restrict P_TY
		= p_reg ? aclty_find(p_reg->m_ty, h_ctx) : 0;

	*rdwr_ty_mid = 0;

	if(!P_TY)
		return 0;

	switch((aclspv_wrd_t)P_TY->m_class) {
		case ACLTY_ARR:
			return P_TY->m_info.m_arr.m_type;

		case ACLTY_PTR:
		{
			const aclty_t* const ae2f_restrict P_POI
				= aclty_find(P_TY->m_info.m_ptr.m_type, h_ctx);

			ae2f_ifezerr(P_POI)
				return 0;

			/** pointer to an array: one level, the pointee's element. */
			if(P_POI->m_class == ACLTY_ARR)
				return P_POI->m_info.m_arr.m_type;

			/** pointer to the wrapped-runtime-array struct: two levels.
			 * member 0 only — the idiom is `struct { T[] }`; any other
			 * member layout is not an array wrapper. */
			if(P_POI->m_class == ACLTY_STRUCT) {
				const aclty_t* const ae2f_restrict P_MBR0
					= aclty_get_struct_mbr(
							(aclty_t*)P_POI
							, 0
							, h_ctx
							);
				const aclty_t* const ae2f_restrict P_MBR0_TY
					= P_MBR0 ? aclty_find(
							P_MBR0->m_info.m_struct_mbr.m_id_mbr
							, h_ctx
							) : 0;

				if(P_MBR0_TY && P_MBR0_TY->m_class == ACLTY_ARR) {
					*rdwr_ty_mid = P_MBR0->m_info.m_struct_mbr.m_id_mbr;
					return P_MBR0_TY->m_info.m_arr.m_type;
				}
			}

			/** pointer to a plain value (or a struct with no array
			 * member 0): one level, the pointee. */
			return P_TY->m_info.m_ptr.m_type;
		}

		default:
			return 0;
	}
}

/**
 * the pointer VALUE carried by a register node — the SPIR-V id holding an
 * address, as opposed to m_addr which yields the address OF the register's
 * own storage.
 *
 * - ALIAS2 view: the view *is* an address (it owns no storage), so its
 *   m_addr hook is the value: one appended OpAccessChain, a freshly minted
 *   id. EMIT-TIME, APPENDING, NON-IDEMPOTENT by contract — never cache the
 *   result and never call this twice for one use.
 * - any other register: the register's own value. aclcur_load_reg decides
 *   between emitting an OpLoad (a memory variable) and handing back the
 *   tracked value (a value / a function parameter, whose id is already the
 *   pointer in logical addressing).
 *
 * @return 0 on failure.
 * */
ae2f_inline_f static aclspv_id_t aclexpr_ptrval(
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	const h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);

	ae2f_ifezerr(p_reg) return 0;
	ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG) return 0;

	if(p_reg->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		return p_reg->m_d.m_reg.m_in.m_addr
				? p_reg->m_d.m_reg.m_in.m_addr(c_reg, h_ctx)
				: 0;

	return aclcur_load_reg(c_reg, h_ctx, 0);
}

/** the pointer a dereference lvalue stores THROUGH.
 *
 * `*q += 2` is a compound assignment whose result belongs at the address q
 * holds, not in q's own storage, so the arithmetic carve cannot route it
 * through aclcur_store_reg the way a plain variable lvalue does.
 *
 * @return 0 when the cursor is not a dereference, or the pointer is not
 * available — the caller then falls back to the plain-variable path.
 * */
ae2f_inline_f static aclspv_id_t aclexpr_ptr_of_deref(
		CXCursor	c_lvalue,
		h_aclspv_ctx_t	h_ctx
		) {
	aclexpr_opnd_t OPND;
	aclspv_id_t REG;

	if(clang_getCursorKind(c_lvalue) != CXCursor_UnaryOperator
	|| CXUnaryOperator_Deref != clang_getCursorUnaryOperatorKind(c_lvalue))
		return 0;

	/** the lvalue short-circuited the child cascade, so this is still the
	 * dereference itself: take its operand explicitly. */
	OPND = aclexpr_opnd_first_child(c_lvalue);

	if(!OPND.m_has)
		return 0;

	ae2f_ifezerr(REG = aclexpr_opnd_reg(OPND.m_c, h_ctx))
		REG = 0;

	if(!REG)
		return 0;

	return aclexpr_ptrval(REG, h_ctx);
}

/** the aclty pointer type of a register node, 0 when it is not a pointer. */
ae2f_inline_f static aclspv_id_t aclexpr_ptr_ty(
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	const h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);
	const aclty_t* const ae2f_restrict P_TY
		= p_reg ? aclty_find(p_reg->m_ty, h_ctx) : 0;

	if(!P_TY || P_TY->m_class != ACLTY_PTR)
		return 0;

	return P_TY->m_info.m_ptr.m_type;
}

/** a pointer operand is required here: classify what was actually supplied.
 *
 * every carve that will load or store THROUGH an id must pass this before
 * emitting. the deref-write carve did not, and the result was an OpStore
 * through an unrelated id — a module that is structurally wrong rather than
 * diagnosable, which is the worst outcome of the three available.
 *
 * an array operand currently lands here too and is reported as a plain
 * not-a-pointer: decay has not landed yet, so there is nothing more specific
 * to say. that distinction comes back with array-to-pointer decay.
 *
 * @return 1 when the register's type really is a pointer.
 * */
ae2f_inline_f static aclspv_bool_t aclexpr_ptr_require(
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	const h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);
	const aclty_t* const ae2f_restrict P_TY
		= p_reg ? aclty_find(p_reg->m_ty, h_ctx) : 0;

	if(P_TY && P_TY->m_class == ACLTY_PTR)
		return 1;

	err_puts("[aclexpr] a pointer operand is required here");
	h_ctx->m_err = ACLSPV_COMPILE_NOT_A_POINTER;
	return 0;
}

/**
 * pointer arithmetic (`p + n`, `p - n`, `p - q`, `p += n`, `++p`, `--p`) is
 * a fatal error in the logical addressing model: pointers are plain ids
 * there, with no address to offset. This is a deliberate rejection of the
 * language model, not a missing carve, hence a dedicated error code.
 *
 * gated on ACLSPV_CTX_FLAG0_LOGICAL alone, so the same expression stays
 * available for physical addressing once byte-offset arithmetic lands.
 * the ACLTY_PTR class test is what keeps ordinary integer / float
 * arithmetic from false-positiveing.
 *
 * @return non-zero when c_ty is a pointer and the rejection fired — the
 * caller must then fail its carve.
 * */
ae2f_inline_f static aclspv_bool_t aclexpr_ptr_arith_fatal(
		aclspv_id_t	c_ty,
		h_aclspv_ctx_t	h_ctx
		) {
	const aclty_t* const ae2f_restrict P_TY
		= c_ty ? aclty_find(c_ty, h_ctx) : 0;

	/** an ACLTY_ARR operand is not classified: decay has not landed, so an
	 * array in arithmetic still passes through here rather than being
	 * refused. array-to-pointer decay brings the check back. */
	unless(P_TY && P_TY->m_class == ACLTY_PTR)
		return 0;

	ae2f_ifezerr(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityAddresses))
		return 0;

	err_puts("[aclexpr] pointer arithmetic is not available in the "
			"logical addressing model");
	h_ctx->m_err = ACLSPV_COMPILE_POINTER_ARITHMETIC;
	return 1;
}

#endif
