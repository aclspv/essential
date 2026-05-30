/** @file bind.h */

#ifndef util_bind_h
#define util_bind_h

#include "./wrd.h"

typedef	struct {
	aclspv_wrd_t		m_storage_class;
	aclspv_id_t		m_var_elm_type_id;
	aclspv_id_t		m_var_type_id;
	aclspv_id_t		m_var_id;
	aclspv_wrdcount_t	m_arg_idx;
	aclspv_wrdcount_t	m_entp_idx;
	CXCursor		m_cursor;
}	aclutil_bind_common;

typedef union {
	aclutil_bind_common		m_unified;

	struct {
		aclutil_bind_common	m_unified;

		/** extension */
		aclspv_wrd_t		m_set;
		aclspv_wrdcount_t	m_binding;
	}	m_bindable;

	struct {
		aclutil_bind_common	m_unified;

		/** extension */
		aclspv_wrdcount_t	m_arr_count_id;
	}	m_work;

	struct {
		aclutil_bind_common	m_unified;

		aclspv_wrdcount_t	m_location;
	}	m_io;

	struct {
		/** unused except for m_arg_idx, m_storage_class */
		aclutil_bind_common	m_unified;
		/** as word count */
		aclspv_wrd_t		m_size;

		/** as word count */
		aclspv_wrd_t		m_pad;
	}	
	/** push constant */
	m_pshconst;
} aclutil_bind;

#endif
