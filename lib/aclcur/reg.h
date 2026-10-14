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
	 * whether its value is scoped: it lives in SSA and needs no memory.
	 *
	 * SET at register initialisation for a Function-storage register, and
	 * inherited by every ALIAS2 view of one. such a value is tracked
	 * through the block-local SSA / phi graph, so a non-explicit store is
	 * a pure definition and materialises no OpStore.
	 *
	 * CLEAR for every other register (interface / global / parameter /
	 * workgroup / address-taken): its value is observable outside the SSA
	 * scope, so a store MUST reach memory.
	 *
	 * the store path reads this flag uniformly (see aclcur_store_reg):
	 * only the initialisation varies by register kind, the validation does
	 * not.
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

	/**
	 * this register is a ghost: it anchors a view chain on a pointer VALUE
	 * (m_base) whose referenced storage the register does not own.
	 *
	 * it never emits storage (m_emit_reg == 0, like a view); its m_id is a
	 * fresh compile-time handle, distinct from m_base. all address work is
	 * forwarded through m_base.
	 * */
	ACLCUR_FLAG0_REG_GHOST,

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
 * sets the flag, m_d.m_reg.m_d.m_ref.m_id = c_parent, and links c_reg
 * into c_parent's child-view list (m_id_child_head / m_id_child_nxt).
 *
 * it does NOT force c_parent to be materialized: materialization is
 * liveness-driven (see aclcur_regvar_emit_reg, which keeps a register
 * whose own or whose ALIAS2 descendant's storage is actually referenced
 * by a live load / explicit store). a view that never touches memory
 * leaves no OpVariable behind.
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

/** the pointer a register is anchored on (m_base): the register's own id
 * (m_id) for a real register, or the held pointer VALUE for a ghost. this
 * is the anchor the view chain walk (aclcur_regview_addr) roots on, instead
 * of reading m_id directly. returns 0 on failure. */
aclspv_id_t	aclcur_reg_base(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/** the pointer VALUE carried by a register — the SPIR-V id holding an
 * address, as opposed to m_addr which yields the address OF the register's
 * own storage. the single deref base rule (D7), keyed on the explicit
 * register kind:
 *
 * - ALIAS2 view: the view *is* an address (it owns no storage), so its
 *   m_addr hook is the value: one appended OpAccessChain. EMIT-TIME,
 *   APPENDING, NON-IDEMPOTENT by contract.
 * - ghost: the held pointer value m_base (identity, no opcode).
 * - any other register: aclcur_load_reg decides between emitting an OpLoad
 *   (a memory variable) and handing back the tracked value (a value / a
 *   function parameter, whose id is already the pointer in logical
 *   addressing).
 *
 * resolves an alias first. @return 0 on failure. */
aclspv_id_t aclcur_reg_ptrval(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);


/** does c_reg's storage have to exist in memory — must a store reach it?
 *
 * resolves the ALIAS2 chain to the register owning the storage: a view's
 * policy is its ROOT's, never a snapshot, so an address taken after the
 * view was created still counts. true when the root is not
 * ACLCUR_FLAG0_REG_SCOPED (its value is observable outside SSA) or is
 * ACLCUR_FLAG0_REG_ALLOCATED (its address escaped). conservative (true)
 * when the register or its root cannot be resolved.
 *
 * resolved at emit time by aclcur_regvar_emit_storeof, so the answer is
 * independent of the order in which stores and address-of were carved. */
aclspv_bool_t	aclcur_reg_needs_memory(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/** the kind of a single view step: how an ALIAS2 view derives from its
 * parent. stored explicitly on the view (m_step) rather than derived from
 * the parent's m_ty, which cannot distinguish a pointer value from a
 * pointer slot. */
enum aclcur_regview_step {
	/** struct/union member: append m_id_idx via OpAccessChain. */
	ACLCUR_REGVIEW_STEP_MEMBER = 0,
	/** array/vector element: append m_id_idx via OpAccessChain. */
	ACLCUR_REGVIEW_STEP_ELEMENT,
	/** pointer dereference: rebase to the parent's pointer value; m_id_idx
	 * is unused. */
	ACLCUR_REGVIEW_STEP_DEREF
};

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
	 * parent. the view CHAIN is implicit in the ALIAS2 parent links. */
	aclspv_id_t	m_id_idx;

	/** the step kind (enum aclcur_regview_step) this ALIAS2 register adds
	 * to its parent: MEMBER / ELEMENT append m_id_idx via OpAccessChain;
	 * DEREF rebases to the parent's pointer value (and leaves m_id_idx
	 * unused). stored explicitly rather than derived from the parent's
	 * m_ty — the latter cannot tell a pointer value from a pointer slot. */
	aclspv_wrd_t	m_step;

	/** see enum SpvStorageClass_ */
	aclspv_wrd_t m_storage_class;

	/** the ALIAS2 view of member 0 of the storage the register's m_id
	 * OpVariable was declared with, when that storage is the frontend's
	 * wrapped argument idiom and differs from the register's logical m_ty
	 * (a storage-buffer / workgroup argument: m_ty is `ptr<SC,T>` or the
	 * array `T[]` while the variable is `ptr<SC, struct{ T }>` /
	 * `ptr<SC, struct{ T[] }>`).
	 *
	 * the view's chain IS the pointer value the argument denotes (one
	 * OpAccessChain selecting member 0), and its m_ty is member 0's type —
	 * which is also the intermediate type of a two-level subscript chain.
	 * the register's own emit_load (args.c) and the subscript
	 * classification (aclexpr_sub_elm_ty) read it; every logical-type
	 * consumer keeps reading m_ty.
	 *
	 * 0 means "no wrapper": the register's address is its m_id verbatim
	 * (an Input/Output argument, or any plain register). */
	aclspv_id_t	m_id_native;

	/** the pointer a register is anchored on, read by the chain walk
	 * (aclcur_regview_addr / aclcur_reg_base) in place of the register's
	 * own id.
	 *
	 * - real register (var / parameter / plain): 0, an identity sentinel
	 *   meaning "the register's own id (m_id) IS its base". it is stored as
	 *   a sentinel rather than a copy because m_id is reassigned to a
	 *   shared OpVariable result id by the entry-point argument idiom
	 *   (aclemit/fndecl/entp/args.c); a copy would go stale.
	 * - ghost register (ACLCUR_FLAG0_REG_GHOST): nonzero, the held pointer
	 *   VALUE this register is a view over; distinct from m_id.
	 *
	 * see aclcur_reg_base(). */
	aclspv_id_t	m_base;
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
aclspv_id_t aclcur_cfg_reg1(
		aclspv_id_t		c_varid_opt,
		CXCursor		c_cur,
		aclspv_id_t		c_ty,
		aclspv_wrd_t		c_storage_class,
		const aclcur_reg_t*	c_in_opt,
		h_aclspv_ctx_t		h_ctx
		);

ae2f_inline_f static aclspv_id_t aclcur_cfg_reg(
		CXCursor		c_cur,
		aclspv_id_t		c_ty,
		aclspv_wrd_t		c_storage_class,
		const aclcur_reg_t*	c_in_opt,
		h_aclspv_ctx_t		h_ctx
		) {
	return aclcur_cfg_reg1(0, c_cur, c_ty, c_storage_class, c_in_opt, h_ctx);
}

#endif
