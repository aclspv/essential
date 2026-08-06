#ifndef	aclutil_cxtp_common_h
#define	aclutil_cxtp_common_h

#include <aclspv.h>
#include <aclconst/specsize.h>
#include <util/wrd.h>

enum ACLUTIL_CXTYPE_ {
	/** 
	 * SHOULD BE ZERO. 
	 * */
	ACLUTIL_CXTYPE_RESERVED = 0,
	ACLUTIL_CXTYPE_UNIT,
	ACLUTIL_CXTYPE_FUNC,
	ACLUTIL_CXTYPE_FUNC_NOPRM,
	ACLUTIL_CXTYPE_VECTOR,
	ACLUTIL_CXTYPE_ARRAY,
	ACLUTIL_CXTYPE_PTR,
	ACLUTIL_CXTYPE_STRUCT,
	ACLUTIL_CXTYPE_STRUCT_DCR,
	ACLUTIL_CXTYPE_STRUCT_MBR
};

enum ACLSPV_CXTP_STRUCT_DECORATION_FIELD0 {
	ACLSPV_CXTP_STRUCT_DECORATION_FIELD0_BLOCK = 1
};

/** @see enum ACLSPV_CXTP_STRUCT_DECORATION_FIELD0 */
typedef aclspv_wrd_t B_aclspv_cxtp_struct_decoration_field0_t;

typedef struct {
	B_aclspv_cxtp_struct_decoration_field0_t	m_field0;
} aclspv_cxtp_struct_decoration;

typedef struct {
	aclspv_wrdcount_t	m_offset;
} aclspv_cxtp_struct_memdecoration;

enum {
	MAX_ACLTYPE_OPRND	= 5
};

typedef struct {
	aclspv_id_t		m_type_id;

	/** constant value id */
	aclconst_specsz		m_sizeof;

	/** constant value */
	aclspv_wrd_t		m_alignof;

	enum ACLUTIL_CXTYPE_ 	m_class;

	union {
		struct {
			aclspv_id_t	m_type;
			aclspv_id_t	m_num_id;
			aclspv_id_t	m_decorate_stride;
		} m_arr;

		struct {
			aclspv_id_t	m_type;
			aclspv_wrd_t	m_num;
		} m_vec;

		struct {
			aclspv_id_t		m_type;
			enum SpvStorageClass_	m_storage;
		} m_ptr;

		struct {
			CXType			m_type;
			CXCursor		m_cursor;
		} m_fn;

		struct {
			aclspv_id_t	m_ret_id;
		} m_fn_no_proto;

		struct {
			aclspv_id_t				m_mem_id;
			aclspv_cxtp_struct_decoration		m_decoration;
			aclspv_cxtp_struct_memdecoration	m_mem_decoration;

			aclspv_wrd_t				m_dcr, m_mbr, m_num_mbr;
			aclspv_wrd_t				m_num_dcr;
			aclspv_wrd_t				m_mem_idx;
			aclspv_wrd_t				m_mem_num;
		} m_struct;

		struct {
			aclspv_id_t	m_id_mbr;
			aclspv_wrd_t	m_idx;
			aclspv_wrd_t	m_nxt;
			aclspv_wrd_t	m_dcr, m_num_dcr;
		} m_struct_mbr;

		struct {
			/** doublely linked list since i am lazy */
			aclspv_wrd_t	m_nxt, m_prv, m_op, m_num_oprnd;
			aclspv_wrd_t	m_oprnd[MAX_ACLTYPE_OPRND];
		} m_struct_decor, m_struct_mem_dcr;
	} m_info;	/** must be plain old buffer (pob) */
} aclutil_cxtp;

#endif
