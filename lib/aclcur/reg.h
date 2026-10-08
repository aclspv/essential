#ifndef	aclcur_reg_h
#define aclcur_reg_h

#include "./flag0.h"

#include <aclconst/specsize.h>
#include <util/literal.h>

/** extention of ACLCUR_FLAG0_OFF_ */
enum ACLCUR_FLAG0_REG_ {
	/** 
	 * its value is tracked for current branch.
	 * no actaul "loading" is required.
	 * */
	ACLCUR_FLAG0_REG_TRACKED	= ACLCUR_FLAG0_MAX,

	/** 
	 * it has history to be used before, but currently not used at all.
	 * which is not visible 
	 *
	 * { int something_like_that; } // so from here i can't see a thing.
	 *
	 * having this flag on should mean its ownership is safe and harmless to move.
	 * */
	ACLCUR_FLAG0_REG_RESERVED,

	/**
	 * this register is an alias of another register.
	 *
	 * the register it aliases is stored at m_d.m_reg.m_d.m_ref.m_id.
	 * it does not own this memory-register really:
	 *
	 * - an alias register is a compile-time indirection only.
	 *   it is never emitted / allocated, and its own value tracking
	 *   (m_val, TRACKED, CHANGED, ALLOCATED, ...) is never consulted.
	 *
	 * - it IS reference-tracked: a STORE/LOAD node created through it
	 *   increments the alias's m_num_ref alongside the resolved
	 *   owner's, so the emission layer sees the alias itself is used.
	 *
	 * - all operations targeting it are redirected to the register it
	 *   resolves to, see aclcur_reg_resolve().
	 *   therefore, no LOAD/STORE node ever references an alias register.
	 *
	 * - the resolved owner is allocated, as any other register, through
	 *   its own interface by the emission layer — the owner's kind
	 *   decides the emitted form (variable, parameter, pointee, ...).
	 * */
	ACLCUR_FLAG0_REG_ALIAS,

	/**
	 * this register is a typed view of another register's storage.
	 * things could be:
	 * - member of a struct/union
	 * - element of an array
	 * - dereference of a pointer with smaller types than original
	 *
	 * unlike ACLCUR_FLAG0_REG_ALIAS, a view IS an operation subject:
	 * it has its own type (m_ty) and its own interface callbacks
	 * (m_in.m_addr, m_in.m_load, m_in.m_store, ...) which compute
	 * parent-relative access. LOAD/STORE/PHI nodes may reference a
	 * view directly, and aclcur_reg_resolve() stops at it.
	 *
	 * the storage is owned by the register at m_d.m_reg.m_d.m_ref.m_id,
	 * linked via aclcur_reg_alias2().
	 *
	 * tracking rules:
	 * - operations on a view respect the parent's storage interface;
	 * - writing a view invalidates the tracking of the view itself,
	 *   of every ALIAS2 ancestor (aggregate no longer matches memory)
	 *   and of every ALIAS2 descendant (sub-view of the written region),
	 *   see aclcur_reg_untrack();
	 * - writing the parent invalidates every descendant view likewise.
	 * */
	ACLCUR_FLAG0_REG_ALIAS2,

	/**
	 * whether its value is scoped.
	 * values will be NOT explicitly stored when out of scope.
	 * */
	ACLCUR_FLAG0_REG_SCOPED,

	/**
	 * its value has been changed to different value than first on current branch.
	 * */
	ACLCUR_FLAG0_REG_CHANGED,

	/** 
	 * this register is referenced as memory: it should be allocated
	 * (emitted) even though nothing else in the frontend mandates it,
	 * e.g. the address of one's variable is taken.
	 *
	 * allocation is performed by the register's own interface during
	 * emission, which decides the emitted form by the register kind
	 * (variable, parameter, reference pointee, composite member view).
	 * the frontend sets this flag to request the materialization.
	 * */
	ACLCUR_FLAG0_REG_ALLOCATED,

	ACLCUR_FLAG0_REG_MAX
};

aclcur_flag0_tent(ACLCUR_FLAG0_REG_, aclcur_flag0_reg);


/**
 * the concrete interface of a register: how the emission layer
 * materializes it once the substrate has made every store/load
 * reference-known (nodes bound to registers, refcounts).
 *
 * each callback is invoked on the register's own interface, so the
 * register kind decides the emitted form (variable, parameter,
 * reference pointee, composite member view, ...). the three concrete
 * operations, invoked for every reference-known register:
 *
 * - emit / allocate:	m_emit_reg. allocate and emit the register's
 *			storage (OpVariable, OpFunctionParameter,
 *			pointee, parent aggregate, ...) before any of
 *			its nodes is emitted.
 *
 * - explicit load:	m_emit_load. emit the OpLoad of a LOAD node
 *			bound to this register.
 *
 * - explicit store:	m_emit_store. emit the OpStore of a STORE node
 *			bound to this register.
 *
 * m_addr / m_size provide the address and byte-size of the storage
 * the emission callbacks operate on (parent-relative for views).
 * m_load / m_store are the per-register-kind load/store semantics:
 * a pure ALIAS forwards to the register it resolves to, a view
 * computes parent-relative access through its m_addr.
 * */
typedef aclspv_bool_t	aclcur_reg_load_t(aclspv_id_t c_reg, aclspv_id_t c_newid, h_aclspv_ctx_t h_ctx);
typedef aclspv_bool_t	aclcur_reg_store_t(aclspv_id_t c_reg, aclspv_id_t c_val, h_aclspv_ctx_t h_ctx);
typedef aclspv_id_t	aclcur_reg_addr_t(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);
typedef aclconst_specsz	aclcur_reg_size_t(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

typedef aclspv_compile_t	aclcur_reg_emit_t(aclspv_id_t c, h_aclspv_ctx_t h_ctx);

/**
 * resolve the register id to the register id which actually owns
 * the memory-register.
 *
 * follows ACLCUR_FLAG0_REG_ALIAS chain transitively, until a register
 * without that flag is found. c_reg itself is returned when it is not
 * a pure alias.
 *
 * note: an ACLCUR_FLAG0_REG_ALIAS2 view is an operation subject on its
 * own, so it is never flattened through: a pure alias of a view
 * resolves to the view, and the view resolves to itself.
 *
 * returns 0 when c_reg is invalid, not a register, or the alias chain
 * is cyclic / too long.
 * */
ae2f_ccpure aclspv_id_t aclcur_reg_resolve(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/**
 * configure c_reg as an ACLCUR_FLAG0_REG_ALIAS2 typed view of the
 * storage owned by c_parent.
 *
 * sets the flag, m_d.m_reg.m_d.m_ref.m_id = c_parent, links c_reg
 * into c_parent's child-view list (m_id_child_head / m_id_child_nxt),
 * and marks c_parent ACLCUR_FLAG0_REG_ALLOCATED: a view implies its
 * whole ancestor chain must be kept (materialized) even with no live
 * load or explicit store. transitivity holds because every view link
 * goes through this function.
 *
 * the caller sets c_reg's type (m_ty, different from the parent's) and
 * its m_in callbacks (parent-relative access).
 *
 * returns c_reg, or 0 when either id is invalid / not a register.
 * */
aclspv_id_t aclcur_reg_alias2(aclspv_id_t c_reg, aclspv_id_t c_parent, h_aclspv_ctx_t h_ctx);

/**
 * invalidate the per-branch tracking of c_reg's register:
 *
 * - the register itself and every ALIAS2 ancestor of it,
 *   because their aggregate no longer matches memory, and
 * - every ALIAS2 descendant view of it, because a sub-region
 *   of their aggregate was written.
 *
 * clears TRACKED, CHANGED, m_val, m_val_mask.
 * */
void aclcur_reg_untrack(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclcur_reg_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);
aclconst_specsz	aclcur_reg_size(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

typedef struct aclcur_reg {
	struct {
		aclcur_reg_load_t*	m_load;
		aclcur_reg_store_t*	m_store;
		aclcur_reg_addr_t*	m_addr;

		/** m_addr contract, per register kind:
		 *
		 * - var (root): PURE IDENTITY. returns the register's own id — an
		 *   OpVariable / OpFunctionParameter result id is already
		 *   `pointer<SC, T>` to its storage — and has no side effects; it
		 *   may be called any number of times.
		 *
		 * - ALIAS2 view: EMIT-TIME, APPENDING, NON-IDEMPOTENT. each call
		 *   appends one OpAccessChain to the current function body
		 *   (m_section.m_fnimpl) and mints a new result id (h_ctx->m_id++).
		 *   NEVER cache its result and never call it twice for a single
		 *   use: the second call is a second opcode, and re-using one id
		 *   across a dominating re-materialization would violate dominance.
		 *
		 * physical-mode addressing (OpPtrAccessChain with byte offsets)
		 * is out of scope for now. */
		aclcur_reg_size_t*	m_size;

		/** EXPLICIT EMISSIONS */
		aclcur_reg_emit_t	*m_emit_reg, *m_emit_load, *m_emit_load_explicit
					, *m_emit_store, *m_emit_store_explicit;
	} m_in;

	union {
		struct {
			aclspv_id_t m_id;
		} m_ref;
	} m_d;

	/** head of the singly-linked list of this register's ALIAS2 child views. */
	aclspv_id_t	m_id_child_head;

	/** next ALIAS2 sibling view sharing the same parent register. */
	aclspv_id_t	m_id_child_nxt;

	/** the single element/member index this ALIAS2 register adds to its
	 * parent. the view CHAIN is implicit in the ALIAS2 parent links; no
	 * tag is needed — the step kind (struct member vs array element) is
	 * derivable from the parent's m_ty. */
	aclspv_id_t	m_id_idx;

	/** see enum SpvStorageClass_ */
	aclspv_wrd_t m_storage_class;
} aclcur_reg_t;

/**
 * create a register node bound to a cursor.
 *
 * c_cur is the cursor it represents (dedup key), c_ty its type,
 * c_storage_class see enum SpvStorageClass_.
 * when c_in_opt is nonzero its m_in (the emission interface) is copied
 * onto the new register; otherwise the register has no storage of its
 * own (alias / member view) and is skipped by aclcur_emit().
 *
 * returns the new register id, or 0 on failure.
 * */
aclspv_id_t aclcur_cfg_reg(
		CXCursor		c_cur,
		aclspv_id_t		c_ty,
		aclspv_wrd_t		c_storage_class,
		const aclcur_reg_t*	c_in_opt,
		h_aclspv_ctx_t		h_ctx
		);

#endif
