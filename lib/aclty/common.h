#ifndef	aclutil_cxtp_common_h
#define	aclutil_cxtp_common_h

#include <aclspv.h>
#include <aclconst/specsize.h>
#include <util/wrd.h>


enum ACLTY_ {
	/** 
	 * SHOULD BE ZERO. 
	 * */
	ACLTY_RESERVED = 0,
	ACLTY_FN,
	ACLTY_FN_ARG,

	/** vector */
	ACLTY_SIMD,
	ACLTY_ARR,
	ACLTY_PTR,
	ACLTY_STRUCT,
	ACLTY_STRUCT_MBR,
	ACLTY_DCR
};

enum {
	MAX_ACLCXTP_DCR_OPRND	= 5
};

enum ACLTY_FIELD0_ {
	/** 
	 * it is not plain old data.
	 * its size could not be known on compile*/
	ACLTY_FIELD0_NOPOD	= 1
};

enum ACLTY_FIELD0_OFF_ {
	ACLTY_FIELD0_OFF_NOPOD
};

/** see ACLTY_FIELD0_ */
typedef aclspv_wrd_t	aclty_field0_t;

typedef struct {
	union {
		struct {
			aclconst_specsz	m_num;
			aclspv_id_t	m_type;
			aclspv_wrd_t	m_dcr, m_num_dcr;
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
			aclspv_id_t	m_ret_id;
			aclspv_wrd_t	m_num_dcr, m_dcr;
			aclspv_wrd_t	m_num_arg, m_arg;
		} m_fn;

		struct {
			aclspv_id_t	m_prm_id;
			aclspv_wrd_t	m_nxt;
			aclspv_wrd_t	m_idx;
		} m_fn_arg;

		struct {
			aclspv_wrd_t	m_dcr, m_mbr, m_num_mbr;
			aclspv_wrd_t	m_num_dcr;
			aclspv_wrd_t	m_mem_idx;
			aclspv_wrd_t	m_mem_num;
		} m_struct;

		struct {
			aclconst_specsz	m_pad;
			aclspv_id_t	m_id_mbr;
			aclspv_wrd_t	m_idx;
			aclspv_wrd_t	m_nxt;
			aclspv_wrd_t	m_dcr, m_num_dcr;
		} m_struct_mbr;

		struct {
			/** doublely linked list since i am lazy */
			aclspv_wrd_t	m_nxt, m_prv, m_op, m_num_oprnd;
			aclspv_wrd_t	m_oprnd[MAX_ACLCXTP_DCR_OPRND];
		} m_dcr;
	} m_info;	/** must be plain old buffer (pob) */

	/** constant value id */
	aclconst_specsz		m_sizeof;

	aclspv_id_t		m_type_id;
	/** constant value */
	aclspv_wrd_t		m_alignof;

	aclty_field0_t		m_field0;

	enum ACLTY_ 		m_class;
} aclty_t;

#endif
