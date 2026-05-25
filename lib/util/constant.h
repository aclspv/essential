/** @file constant.h */

#ifndef	lib_build_constant_h
#define lib_build_constant_h


#include <ae2f/Keys.h>
#include <ae2f/c90/StdInt.h>

#include "./ctx.h"
#include "./id.h"
#include "aclspv.h"
#include "util/wrd.h"

#include <spirv/unified1/spirv.h>

enum UTIL_CV_NBYTE_ { UTIL_CV_NBYTE_1, UTIL_CV_NBYTE_2, UTIL_CV_NBYTE_4, UTIL_CV_NBYTE_8, UTIL_CV_NBYTE_MAX };
enum UTIL_CV_TY_ { UTIL_CV_TY_SIGNED, UTIL_CV_TY_UNSIGNED, UTIL_CV_TY_FLT, UTIL_CV_TY_MAX };

typedef struct {
	aclspv_wrd_t		m_key_0;
	aclspv_wrd_t		m_key_1;

	/** @brief @see UTIL_CV_TY_, UTIL_CV_NBYTE_ */
	aclspv_id_t		m_const_ids[UTIL_CV_NBYTE_MAX][UTIL_CV_TY_MAX];

	aclspv_id_t		m_const_val_bool;
	aclspv_id_t		m_const_spec_id;
	e_id_default		m_const_spec_type;

	aclspv_id_t		m_repvec;
	aclspv_wrd_t		m_repvec_literal_mask;
} util_constant;

typedef util_constant* ae2f_restrict p_util_constant_t;


aclspv_id_t aclutil_mk_constant_val_id1(
		const u64_least		c_val,
		h_util_ctx_t		h_ctx,
		enum UTIL_CV_NBYTE_	c_idx_nbyte,
		enum UTIL_CV_TY_	c_idx_ty
		);


aclspv_id_t aclutil_mk_constant_bool_id(
		const aclspv_bool_t c_val,
		h_util_ctx_t h_ctx
		);

aclspv_id_t aclutil_mk_constant_val_id(
		const aclspv_wrd_t c_val,
		h_util_ctx_t h_ctx
		);

aclspv_id_t aclutil_mk_constant_val64_id(
		const u64_least c_val, 
		h_util_ctx_t h_ctx
		);

#define	aclutil_mk_constant_val_id(c_val, h_ctx)	aclutil_mk_constant_val_id1(c_val, h_ctx, UTIL_CV_NBYTE_4, UTIL_CV_TY_SIGNED)
#define	aclutil_mk_constant_val64_id(c_val, h_ctx)	aclutil_mk_constant_val_id1(c_val, h_ctx, UTIL_CV_NBYTE_8, UTIL_CV_TY_SIGNED)


aclspv_id_t aclutil_mk_constant_val_f32_id(
		const float c_val, 
		h_util_ctx_t h_ctx
		);


aclspv_id_t aclutil_mk_constant_val_f64_id(
		const double c_val, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_spec_id(
		const aclspv_wrd_t c_key, const aclspv_wrd_t c_val, 
		h_util_ctx_t h_ctx
		);

aclspv_id_t	aclutil_mk_constant_spec64_id(
		const aclspv_wrd_t c_key, 
		const u64_least c_val,
		h_util_ctx_t h_ctx
		);

aclspv_id_t	aclutil_mk_constant_spec_id_f64(
		const aclspv_wrd_t c_key, 
		const double c_val, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_spec_id_f32(
		const aclspv_wrd_t c_key, 
		const float c_val, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_vec32_id(
		const aclspv_wrd_t c_val, 
		h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_arr8_id(const u64_least c_arrcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_arr16_id(const u64_least c_arrcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_arr32_id(const u64_least c_arrcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_arr128_id(const u64_least c_arrcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_struct_id(const aclspv_wrd_t c_arrcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_struct128_id(const aclspv_wrd_t c_veccount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_structprivspec_id(const aclspv_wrd_t c_key, const u64_least c_wrdcount, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_structpriv_id(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_psh_id(const aclspv_wrd_t c_wrdcount, const h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_storage_id(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_id(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_func(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_uniform_id(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_uniformconstant_id(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_workspec_id(const aclspv_wrd_t c_key, const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_work_id(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx);

struct aclutil_mk_constant_repvec_t {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_literal_mask;
};

/** @brief repeating vector */
struct aclutil_mk_constant_repvec_t	aclutil_mk_constant_repvec_id(const aclspv_id_t	c_id_ty, const aclspv_id_t c_val, const aclspv_wrd_t c_literal_mask, h_util_ctx_t h_ctx);


ae2f_unused ae2f_inline_f static aclspv_id_t	aclutil_mk_constant_val_by_enum(const u64_least c_val, h_util_ctx_t h_ctx, enum ID_DEFAULT_ c_ty) {
	switch((uint_leastenum_t)c_ty) {
#define	CASE_NUM(k, nbyte, ty)	case (k): return	\
		aclutil_mk_constant_val_id1(c_val, h_ctx, (nbyte), (ty))

		CASE_NUM(ID_DEFAULT_I8, UTIL_CV_NBYTE_1, UTIL_CV_TY_SIGNED);
		CASE_NUM(ID_DEFAULT_I16, UTIL_CV_NBYTE_2, UTIL_CV_TY_SIGNED);
		CASE_NUM(ID_DEFAULT_I32, UTIL_CV_NBYTE_4, UTIL_CV_TY_SIGNED);
		CASE_NUM(ID_DEFAULT_I64, UTIL_CV_NBYTE_8, UTIL_CV_TY_SIGNED);

		CASE_NUM(ID_DEFAULT_U8, UTIL_CV_NBYTE_1, UTIL_CV_TY_UNSIGNED);
		CASE_NUM(ID_DEFAULT_U16, UTIL_CV_NBYTE_2, UTIL_CV_TY_UNSIGNED);
		CASE_NUM(ID_DEFAULT_U32, UTIL_CV_NBYTE_4, UTIL_CV_TY_UNSIGNED);
		CASE_NUM(ID_DEFAULT_U64, UTIL_CV_NBYTE_8, UTIL_CV_TY_UNSIGNED);

		CASE_NUM(ID_DEFAULT_F16, UTIL_CV_NBYTE_2, UTIL_CV_TY_FLT);
		CASE_NUM(ID_DEFAULT_F32, UTIL_CV_NBYTE_4, UTIL_CV_TY_FLT);
		CASE_NUM(ID_DEFAULT_F64, UTIL_CV_NBYTE_8, UTIL_CV_TY_FLT);

#undef	CASE_NUM

		case ID_DEFAULT_BOOL: return aclutil_mk_constant_bool_id((aclspv_bool_t)c_val, h_ctx);

		default:
			return 0;
	}
}

ae2f_unused ae2f_inline_f static aclspv_id_t	aclutil_mk_constant_ptr_by_enum(const aclspv_wrd_t c_wrdcount, h_util_ctx_t h_ctx, enum SpvStorageClass_ c_class) {
	switch((aclspv_wrd_t)c_class) {
		case SpvStorageClassWorkgroup:
			return aclutil_mk_constant_ptr_work_id(c_wrdcount, h_ctx);
		case SpvStorageClassPrivate:
			return aclutil_mk_constant_ptr_id(c_wrdcount, h_ctx);
		case SpvStorageClassUniform:
			return aclutil_mk_constant_ptr_uniform_id(c_wrdcount, h_ctx);
		case SpvStorageClassStorageBuffer:
			return aclutil_mk_constant_ptr_storage_id(c_wrdcount, h_ctx);
		case SpvStorageClassPushConstant:
			return aclutil_mk_constant_ptr_psh_id(c_wrdcount, h_ctx);

		default:
			ae2f_unreachable();
			return 0;
	}
}

util_constant* aclutil_mk_constant_node(
		const u64_least	c_key,
		h_util_ctx_t	h_ctx
		);
#endif
