#ifndef aclutil_cxtp_h
#define	aclutil_cxtp_h

#include "./wrd.h"
#include "util/ctx.h"

#include <clang-c/Index.h>
#include <ae2f/Keys.h>
#include <spirv/unified1/spirv.h>

enum ACLUTIL_CXTYPE_ {
	ACLUTIL_CXTYPE_UNIT,
	ACLUTIL_CXTYPE_FUNC,
	ACLUTIL_CXTYPE_FUNC_NOPRM,
	ACLUTIL_CXTYPE_VECTOR,
	ACLUTIL_CXTYPE_ARRAY,
	ACLUTIL_CXTYPE_PTR,
	ACLUTIL_CXTYPE_STRUCT_SINGLE
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

typedef struct {
	aclspv_id_t		m_type_id;
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
		} m_struct_single;
	} m_info;	/** must be plain old buffer (pob) */
} aclutil_cxtp;

ae2f_ccconst aclspv_id_t aclutil_mk_cxtp_by_type(const CXType c_ty);
aclspv_id_t aclutil_mk_cxtp_by_type2(
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		);

/** Returns the type id of type */
aclspv_id_t aclutil_mk_cxtp_by_cursor(
		const CXCursor		c_cursor,
		const CXType		c_type,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclutil_mk_cxtp_by_cursor2(
		const CXCursor		c_cursor,
		const h_aclspv_ctx_t	h_ctx
		);

/** array with stride */
aclspv_id_t aclutil_mk_cxtp_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrdcount_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		);

#define	aclutil_mk_cxtp_arr(c_type_elm, c_num_arr, h_ctx)	\
	aclutil_mk_cxtp_arr2(c_type_elm, c_num_arr, 0xFFFFFFFF, h_ctx)

aclspv_id_t aclutil_mk_cxtp_vec(
		const aclspv_id_t	c_type_elm,
		const aclspv_wrd_t	c_num_arr,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclutil_mk_cxtp_ptr(
		const aclspv_id_t		c_type_elm,
		const enum SpvStorageClass_	c_storage_class,
		const h_aclspv_ctx_t		h_ctx
		);

aclspv_id_t aclutil_mk_cxtp_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclutil_mk_cxtp_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclutil_mk_cxtp_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t	
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		);

struct aclutil_cxtp_veccfg_t {
	aclspv_id_t	m_ty_elm;
	aclspv_wrd_t	m_num;
}	aclutil_cxtp_get_veccfg(
		const aclspv_id_t	c_ty_vec,
		const h_aclspv_ctx_t	h_ctx
		);

#endif
