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

struct a_aclspv_ctx {
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


		/**
		 * [emit::body] boot scale
		 * */
		x_aclspv_vec	m_v1;

		/**
		 * [emit::body] command counter stack
		 * */
		x_aclspv_vec	m_v2;
	}	m_tmp;

	aclspv_compile_t	m_err;

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
	aclspv_wrd_t		m_ctx_sz;
	void*			m_ctx_base;
	aclspv_usrcfg_t		m_usrcfg;
};

#endif
