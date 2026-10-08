#ifndef aclexpr_op_h
#define aclexpr_op_h

#include <aclty/vec.h>
#include <util/literal.h>

enum ACLEXPR_OPBINCFG_ {
	ACLEXPR_OPBINCFG_NUMOPRND,

	/** @see ./literal.h */

	ACLEXPR_OPBINCFG_SPECCONSTANTOP,
	ACLEXPR_OPBINCFG_RESTYID,
	ACLEXPR_OPBINCFG_RESID,
	ACLEXPR_OPBINCFG_OPCODE,
	ACLEXPR_OPBINCFG_END
};

enum ACLEXPR_OP_ {
	ACLEXPR_OP1_EXPLICIT_CAST,
	ACLEXPR_OP1_POSTINC,
	ACLEXPR_OP1_PREINC,
	ACLEXPR_OP1_POSTDEC,
	ACLEXPR_OP1_PREDEC,
	/** composite member read (`s.b`): OpCompositeExtract. result type is
	 * the member type (RESTYID); the member index rides in the scale's
	 * extra[1] literal. */
	ACLEXPR_OP1_MEMBERREAD,

	/** address-of (`&x`): emits no instruction of its own. the result is
	 * the operand register's m_addr — identity for a var (its
	 * OpVariable / OpFunctionParameter result id), one freshly appended
	 * OpAccessChain for a view. RESTYID is resolved at carve time: a
	 * pointer type needs the operand's storage class. */
	ACLEXPR_OP1_ADDRESS,
	/** dereference read (`*p`): OpLoad from the operand's pointer value. */
	ACLEXPR_OP1_DEREF,

	ACLEXPR_OP2_ADD,
	ACLEXPR_OP2_ADDASS,
	ACLEXPR_OP2_SUB,
	ACLEXPR_OP2_SUBASS,
	ACLEXPR_OP2_MUL,
	ACLEXPR_OP2_MULASS,
	ACLEXPR_OP2_REM,
	ACLEXPR_OP2_REMASS,
	ACLEXPR_OP2_DIV,
	ACLEXPR_OP2_DIVASS,

	ACLEXPR_OP2_SHL,
	ACLEXPR_OP2_SHLASS,
	ACLEXPR_OP2_SHR,
	ACLEXPR_OP2_SHRASS,

	ACLEXPR_OP2_ASSIGN,
	/** composite member write (`s.b = x`): OpCompositeInsert into the base
	 * register, then store. the RHS id rides in operand slot 1; the base
	 * register and the member index are re-derived from the MemberRefExpr
	 * lvalue cursor (extra[0].m_cursor) at carve time. */
	ACLEXPR_OP2_MEMBERWRITE,
	/** dereference write (`*p = x`): OpStore through the pointer of the
	 * deref lvalue, which is re-derived from the lvalue cursor
	 * (extra[0].m_cursor) exactly like the member-write carve does. the
	 * RHS id rides in operand slot 1. */
	ACLEXPR_OP2_DEREFWRITE,
	/** subscript as an assignment target (`a[i] = v`): build the view, then
	 * OpStore through it. routed from the assign lane the way MEMBERWRITE
	 * is, and — like it — an op2 because the assign scale carries the RHS
	 * in operand slot 1. */
	ACLEXPR_OP2_SUBSCRIPTWRITE,
	/** array subscript (`a[i]`): an ALIAS2 view of the array register at
	 * the index, read back with OpLoad. two operands — the array and the
	 * index — so it dispatches through the op2 carve chain, not op1. */
	ACLEXPR_OP2_SUBSCRIPT,

	ACLEXPR_OP2_SLT,
	ACLEXPR_OP2_SLE,
	ACLEXPR_OP2_SGT,
	ACLEXPR_OP2_SGE,
	ACLEXPR_OP2_EQ,
	ACLEXPR_OP2_NE,

	ACLEXPR_OP2_AND,
	ACLEXPR_OP2_AND_L,
	ACLEXPR_OP2_AND_A,
	ACLEXPR_OP2_OR,
	ACLEXPR_OP2_OR_L,
	ACLEXPR_OP2_OR_A,
	ACLEXPR_OP2_XOR,
	ACLEXPR_OP2_XOR_A,

	/** N-ary: variable operand count (see OPN_ stages) */
	ACLEXPR_OPN_INITLIST
};

typedef struct {
	aclspv_wrd_t	m_type_original;
	aclspv_wrd_t	m_mask_literal;
	aclutil_literal	m_literal;
	CXCursor	m_cursor;
	aclspv_wrd_t	m_state0;

	union {
		struct {
			aclspv_id_t	m_parent;
		} m_bool1;

		/** set by the deref read carve: the pointer the load went
		 * through. a compound assignment whose lvalue is a dereference
		 * (`*q += 2`) must store back through that same pointer, and the
		 * lvalue cursor it would re-derive is gone by then — the visit
		 * cascade overwrites extra[0].m_cursor with each cursor on the way
		 * down. m_extra survives, m_cursor does not. */
		struct {
			aclspv_id_t	m_ptr_id;
		} m_deref;
	} m_extra;
} aclexpr_opbincfg_extra;

enum ACLEXPR_OPCFGX_STATE0_ {
	ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED	= 0x1,
	/** m_extra.m_deref.m_ptr_id is valid: this value was loaded through the
	 * pointer recorded there. */
	ACLEXPR_OPCFGX_STATE0_DEREF_LOADED	= 0x2,
	/** `&a[i]` — this subscript is the operand of an address-of, so its
	 * result is the element's address, not a load of the element.
	 *
	 * the visit lane sets it on the subscript scale's own extra[0] on the way
	 * down, and the subscript carve re-sets it on the extra it hands back, so
	 * it reaches the address carve riding PRV_EXTRA — by which point
	 * extra[0].m_cursor has been rewritten by the cascade to name the ARRAY,
	 * and is of no use to tell the two apart. */
	ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM	= 0x4
};

/**
 * @brief type alias for `enum UTIL_OPBIN2_`
 * */
typedef aclspv_wrd_t aclexpr_opbin2_t;

#endif
