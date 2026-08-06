/** @file constant.h */

#ifndef	lib_build_constant_h
#define lib_build_constant_h


#include <ae2f/Keys.h>
#include "./wrd.h"
#include "./id.h"


enum ACLUTIL_CV_NBYTE_ { ACLUTIL_CV_NBYTE_1, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_NBYTE_MAX };
enum ACLUTIL_CV_TY_ { ACLUTIL_CV_TY_SIGNED, ACLUTIL_CV_TY_UNSIGNED, ACLUTIL_CV_TY_FLT, ACLUTIL_CV_TY_MAX };

ae2f_ccconst static ae2f_inline_f enum ACLSPV_ID_ s_aclutil_cvty(
		enum ACLUTIL_CV_TY_ c_ty,
		enum ACLUTIL_CV_NBYTE_ c_nby
		) {
	const enum ACLSPV_ID_ MAP[ACLUTIL_CV_TY_MAX][ACLUTIL_CV_NBYTE_MAX] = {
		{ ACLSPV_ID_I8, ACLSPV_ID_I16, ACLSPV_ID_I32, ACLSPV_ID_I64 },
		{ ACLSPV_ID_U8, ACLSPV_ID_U16, ACLSPV_ID_U32, ACLSPV_ID_U64 },
		{ 0, ACLSPV_ID_F16, ACLSPV_ID_F32, ACLSPV_ID_F64 }
	};

	return MAP[c_ty][c_nby];
}

typedef struct {
	aclspv_id_t	m_val, m_ty;
} aclutil_specconstant_val_t;

typedef struct {
	aclspv_wrd_t		m_key_0;
	aclspv_wrd_t		m_key_1;

	/** @brief @see UTIL_CV_TY_, UTIL_CV_NBYTE_ */
	aclspv_id_t		m_const_ids[ACLUTIL_CV_NBYTE_MAX][ACLUTIL_CV_TY_MAX];

	aclspv_id_t		m_const_val_bool;

	aclutil_specconstant_val_t	m_spec_constant;

	aclspv_id_t		m_repvec;
	aclspv_wrd_t		m_repvec_literal_mask;
} aclutil_constant;

typedef aclutil_constant* ae2f_restrict p_aclutil_constant_t;


aclspv_id_t aclutil_mk_constant_val_id1(
		const u64_least		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLUTIL_CV_NBYTE_	c_idx_nbyte,
		enum ACLUTIL_CV_TY_	c_idx_ty
		);

aclutil_specconstant_val_t ae2f_ccpure aclutil_get_specconstant_val_id(
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_idx_specconst
		);

aclutil_specconstant_val_t aclutil_mk_specconstant_val_id(
		const u64_least		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLUTIL_CV_NBYTE_	c_idx_nbyte,
		enum ACLUTIL_CV_TY_	c_idx_ty,
		aclspv_wrd_t		c_idx_specconst
		);

aclspv_id_t aclutil_mk_constant_bool_id(
		const aclspv_bool_t c_val,
		h_aclspv_ctx_t h_ctx
		);

aclspv_id_t aclutil_mk_constant_val_id(
		const aclspv_wrd_t c_val,
		h_aclspv_ctx_t h_ctx
		);

aclspv_id_t aclutil_mk_constant_val64_id(
		const u64_least c_val, 
		h_aclspv_ctx_t h_ctx
		);

#define	aclutil_mk_constant_val_id(c_val, h_ctx)	aclutil_mk_constant_val_id1(c_val, h_ctx, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_SIGNED)
#define	aclutil_mk_constant_val64_id(c_val, h_ctx)	aclutil_mk_constant_val_id1(c_val, h_ctx, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_SIGNED)


aclspv_id_t aclutil_mk_constant_val_f32_id(
		const float c_val, 
		h_aclspv_ctx_t h_ctx
		);


aclspv_id_t aclutil_mk_constant_val_f64_id(
		const double c_val, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_spec_id(
		const aclspv_wrd_t c_key, const aclspv_wrd_t c_val, 
		h_aclspv_ctx_t h_ctx
		);

aclspv_id_t aclutil_mk_specconstant_bool_id(
		const aclspv_wrd_t	c_idx_spec,
		const aclspv_bool_t	c_val,
		h_aclspv_ctx_t h_ctx
		);

aclspv_id_t	aclutil_mk_constant_spec64_id(
		const aclspv_wrd_t c_key, 
		const u64_least c_val,
		h_aclspv_ctx_t h_ctx
		);

aclspv_id_t	aclutil_mk_constant_spec_id_f64(
		const aclspv_wrd_t c_key, 
		const double c_val, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_spec_id_f32(
		const aclspv_wrd_t c_key, 
		const float c_val, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_vec32_id(
		const aclspv_wrd_t c_val, 
		h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_arr8_id(const u64_least c_arrcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_arr16_id(const u64_least c_arrcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_arr32_id(const u64_least c_arrcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_arr128_id(const u64_least c_arrcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_struct_id(const aclspv_wrd_t c_arrcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_struct128_id(const aclspv_wrd_t c_veccount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_structprivspec_id(const aclspv_wrd_t c_key, const u64_least c_wrdcount, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_structpriv_id(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_psh_id(const aclspv_wrd_t c_wrdcount, const h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_storage_id(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_id(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_func(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_uniform_id(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_uniformconstant_id(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);

aclspv_id_t	aclutil_mk_constant_ptr_workspec_id(const aclspv_wrd_t c_key, const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);


aclspv_id_t	aclutil_mk_constant_ptr_work_id(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx);

struct aclutil_mk_constant_repvec_t {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_literal_mask;
};

/** @brief repeating vector */
struct aclutil_mk_constant_repvec_t	aclutil_mk_constant_repvec_id(const aclspv_id_t	c_id_ty, const aclspv_id_t c_val, const aclspv_wrd_t c_literal_mask, h_aclspv_ctx_t h_ctx);


ae2f_unused ae2f_inline_f static aclspv_id_t	aclutil_mk_constant_val_by_enum(const u64_least c_val, h_aclspv_ctx_t h_ctx, enum ACLSPV_ID_ c_ty) {
	switch((uint_leastenum_t)c_ty) {
#define	CASE_NUM(k, nbyte, ty)	case (k): return	\
		aclutil_mk_constant_val_id1(c_val, h_ctx, (nbyte), (ty))

		CASE_NUM(ACLSPV_ID_I8, ACLUTIL_CV_NBYTE_1, ACLUTIL_CV_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I16, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I32, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I64, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_SIGNED);

		CASE_NUM(ACLSPV_ID_U8, ACLUTIL_CV_NBYTE_1, ACLUTIL_CV_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U16, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U32, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U64, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_UNSIGNED);

		CASE_NUM(ACLSPV_ID_F16, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_TY_FLT);
		CASE_NUM(ACLSPV_ID_F32, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_FLT);
		CASE_NUM(ACLSPV_ID_F64, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_FLT);

#undef	CASE_NUM

		case ACLSPV_ID_BOOL: return aclutil_mk_constant_bool_id((aclspv_bool_t)c_val, h_ctx);

		default:
			return 0;
	}
}

ae2f_unused ae2f_inline_f static aclspv_id_t	aclutil_mk_specconstant_val_by_enum(
		const aclspv_wrd_t c_key,
		const u64_least c_val,
		h_aclspv_ctx_t h_ctx,
		enum ACLSPV_ID_ c_ty
		) {
	switch((uint_leastenum_t)c_ty) {
#define	CASE_NUM(k, nbyte, ty)	case (k): return	\
		aclutil_mk_specconstant_val_id(c_val, h_ctx, (nbyte), (ty), c_key).m_val

		CASE_NUM(ACLSPV_ID_I8, ACLUTIL_CV_NBYTE_1, ACLUTIL_CV_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I16, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I32, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I64, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_SIGNED);

		CASE_NUM(ACLSPV_ID_U8, ACLUTIL_CV_NBYTE_1, ACLUTIL_CV_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U16, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U32, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U64, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_UNSIGNED);

		CASE_NUM(ACLSPV_ID_F16, ACLUTIL_CV_NBYTE_2, ACLUTIL_CV_TY_FLT);
		CASE_NUM(ACLSPV_ID_F32, ACLUTIL_CV_NBYTE_4, ACLUTIL_CV_TY_FLT);
		CASE_NUM(ACLSPV_ID_F64, ACLUTIL_CV_NBYTE_8, ACLUTIL_CV_TY_FLT);

#undef	CASE_NUM
		default:
			return 0;
	}
}

ae2f_unused ae2f_inline_f static aclspv_id_t	aclutil_mk_constant_ptr_by_enum(const aclspv_wrd_t c_wrdcount, h_aclspv_ctx_t h_ctx, enum SpvStorageClass_ c_class) {
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

aclutil_constant* aclutil_mk_constant_node(
		const u64_least	c_key,
		h_aclspv_ctx_t	h_ctx
		);
#endif
