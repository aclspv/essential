/** this is not necessarily public. */

#ifndef	aclspv_ctx_ctx_h
#define aclspv_ctx_ctx_h

#include <aclspv/wrd.h>
#include <aclspv/id.h>
#include <aclspv/usrcfg.h>
#include <aclspv/enum/err.h>
#include <aclspv/enum/id.h>
#include <aclspv/vec.h>
#include <clang-c/Index.h>

typedef struct aclval	aclval_t;
typedef struct aclty	aclty_t;
typedef struct aclcur	aclcur_t;
typedef aclspv_wrd2_t	aclcur_lbl_dfs_visit_t;
typedef struct aclspv_entp	aclspv_entp_t;
typedef struct aclspv_fn	aclspv_fn_t;

/** internal: which emission phase has run. see aclspv_ctx_compile / build. */
enum ACLSPV_CTX_PHASE_ {
	ACLSPV_CTX_PHASE_NONE = 0,
	ACLSPV_CTX_PHASE_COMPILE,
	ACLSPV_CTX_PHASE_BUILD
};

struct a_aclspv_ctx {
	/** sub-ctx for something not should be emitted */
	x_aclspv_ctx* ae2f_restrict m_ctx_slave;

	/** cache for elements related to unsigned integer constants [lib_build_constant] */
	x_aclspv_vec		m_constant_cache;

	/** 
	 * @brief
	 * cache for cursors for parsing one function 
	 * for its use see util/cursor.h
	 *
	 * [util_cursor]
	 *
	 * @details
	 * reset on every function
	 * */
	x_aclspv_vec		m_cursors;

	x_aclspv_vec		m_cur_lbl_dfs;
	x_aclspv_vec		m_cur_lbl_dfs_visit;

	/** 
	 * @brief
	 * cache for complex types which needs to be stored somewhere
	 * for its use see util/cxtp.h
	 *
	 * [aclutil_cxtp]
	 * */
	x_aclspv_vec		m_type_uniques;

	struct {
		/** 
		 * [*]		general-purpose true temporary vector
		 * */
		x_aclspv_vec	m_v0;
	}	m_tmp;

	/**
	 * @brief	expression evaluation scratch.
	 *
	 * an expression evaluator owns one boot scale (see util/scale.h) and one
	 * command-counter stack. a statement expression evaluates statements
	 * while an expression is still being evaluated, so the two evaluators
	 * must not share this scratch: the outer pair is parked on the `_stck`
	 * vectors and the inner evaluator gets a fresh pair of its own. each
	 * stack carries its own count so it can be disposed in LIFO order.
	 * */
	struct {
		/** [emit::body] boot scale */
		x_aclspv_vec	m_bootscale;

		/** [emit::body] command counter stack */
		x_aclspv_vec	m_cmdcountstck;

		/** parked boot scales of the suspended (outer) evaluators */
		x_aclspv_vec	m_bootscale_stck;
		aclspv_wrd_t		m_num_bootscale_stck;

		/** parked command-counter stacks of the suspended evaluators */
		x_aclspv_vec	m_cmdcountstck_stck;
		aclspv_wrd_t		m_num_cmdcountstck_stck;
	}	m_body;

	aclspv_compile_t	m_err;

	/** internal: enum ACLSPV_CTX_PHASE_. 0 on a fresh context. */
	aclspv_wrd_t		m_phase;

	/** 
	 * @brief
	 * number of `m_cursors`.	\n
	 * for its use see util/cursor.h
	 *
	 * @details
	 * reset on every function
	 * */
	aclspv_wrdcount_t	m_num_cursor;

	/** @see `m_type_uniques` */
	aclspv_wrdcount_t	m_num_type_uniques;
	aclspv_wrdcount_t	m_num_constant_cache;

	/**
	 * @var		m_id
	 * @brief	id
	 * */
	aclspv_id_t		m_id;

	struct {
		aclspv_wrdcount_t
			m_capability,
			m_extinst,
			m_ext,
			m_entp,
			m_execmode,
			m_name,
			m_name_symbol,
			m_decorate,
			m_types,
			m_vars,
			m_fndef,
			m_fnimpl;
	}
	/** 
	 * @brief	DO NOT EDIT THIS OUTSIDE THIS LIBRARY IF YOU DO NOT HAVE ENOUGH CONTEXT 
	 * */
	m_count;

	struct {
		aclspv_wrdcount_t
			m_num_fn,
			m_num_entp;

		/**
		 * @var		m_fn
		 * @brief	normal functions
		 * @see		lib_build_fn_t
		 * */
		x_aclspv_vec	m_fn;

		/**
		 * @var		m_entp
		 * @brief	entry functions
		 * @see		lib_build_entp_t
		 * */
		x_aclspv_vec	m_entp;

		/**
		 * @var		m_fn_name
		 * @brief	flat byte storage for the names of `m_fn`: each symbol
		 *		owns [m_idx_name, m_idx_name + m_len_name).
		 *		no cursor is kept: names are the durable identity and a
		 *		4-byte mark points back into a live translation unit.
		 * @see		aclsym_fn_t
		 * */
		x_aclspv_vec	m_fn_name;

		/** number of bytes in `m_fn_name`. */
		aclspv_wrdcount_t	m_num_fn_name;
	}			m_fnlist;

	/**
	 * @var		m_section
	 * @brief	each section for m_ret
	 * */
	struct {
		x_aclspv_vec
			m_capability,
			m_extinst,
			m_ext,
			m_entp,
			m_execmode,
			m_name,
			m_name_symbol,
			m_decorate,
			m_types,
			m_vars,
			m_fndef,
			m_fnimpl;
	}			m_section;

	aclspv_id_t		m_id_defaults[ACLSPV_ID_END];

	/** push constant should be singleton. */
	aclspv_wrd_t		m_push_size_ty_pad;
	aclspv_wrd_t		m_num_push_constant_func;

	CXCursorVisitor		m_work_nxt;
	CXClientData		m_work_data;

	/**
	 * @brief	GNU statement expression (`({ ... })`) emitter.
	 *
	 * set by aclemit while a function body is being emitted: it carves the
	 * statement list of c_stmtexpr into the current control-flow context and
	 * returns the value id of the block's last expression, or 0 on failure.
	 *
	 * null outside body emission, where a statement expression would mean
	 * the expression is being evaluated as a constant and cannot be carved.
	 * */
	aclspv_id_t		(*m_stmtexpr)(CXCursor c_stmtexpr, struct a_aclspv_ctx* h_ctx);
	aclspv_wrd_t		m_ctx_sz;
	void*			m_ctx_base;
	aclspv_usrcfg_t		m_usrcfg;
};

#endif
