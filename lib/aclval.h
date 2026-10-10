#ifndef	aclval_h
#define aclval_h

#include <util/wrd.h>
#include <aclctx/id.h>


enum ACLVAL_NBYTE_ {
	ACLVAL_NBYTE_1,
	ACLVAL_NBYTE_2,
	ACLVAL_NBYTE_4,
	ACLVAL_NBYTE_8,
	ACLVAL_NBYTE_MAX
};
enum ACLVAL_TY_ {
	ACLVAL_TY_SIGNED,
	ACLVAL_TY_UNSIGNED,
	ACLVAL_TY_FLT,
	ACLVAL_TY_MAX
};

ae2f_ccconst static ae2f_inline_f enum ACLSPV_ID_ s_aclval_ty(
		enum ACLVAL_TY_ c_ty,
		enum ACLVAL_NBYTE_ c_nby
		) {
	const enum ACLSPV_ID_ MAP[ACLVAL_TY_MAX][ACLVAL_NBYTE_MAX] = {
		{ ACLSPV_ID_I8, ACLSPV_ID_I16, ACLSPV_ID_I32, ACLSPV_ID_I64 },
		{ ACLSPV_ID_U8, ACLSPV_ID_U16, ACLSPV_ID_U32, ACLSPV_ID_U64 },
		{ 0, ACLSPV_ID_F16, ACLSPV_ID_F32, ACLSPV_ID_F64 }
	};

	return MAP[c_ty][c_nby];
}

typedef struct {
	aclspv_id_t	m_val, m_ty;
} aclval_spec_t;

typedef struct {
	aclspv_wrd_t		m_key_0;
	aclspv_wrd_t		m_key_1;

	/** @brief @see UTIL_CV_TY_, UTIL_CV_NBYTE_ */
	aclspv_id_t		m_const_ids[ACLVAL_TY_MAX][ACLVAL_NBYTE_MAX];

	aclspv_id_t		m_const_val_bool;
	
	/**
	 * SpvStorageClass
	 * m_key_0(lsb): bind index
	 * m_key_1(hsb): descriptor set
	 *
	 * its init value should be SpvStorageClassMax
	 * */
	aclspv_wrd_t		m_set_bind_storage_class;

	/**
	 * m_key_0 (lsb): descriptor set
	 *
	 * its last bind index from descriptor set
	 * */
	aclspv_wrd_t		m_set_bind_last_idx;

	aclval_spec_t	m_spec_constant;

	aclspv_id_t		m_repvec;
	aclspv_wrd_t		m_repvec_literal_mask;
} aclval_t;

typedef aclval_t* ae2f_restrict p_aclval_t;


aclspv_id_t aclval_cfg(
		const aclspv_wrd2_t		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLVAL_NBYTE_	c_idx_nbyte,
		enum ACLVAL_TY_	c_idx_ty
		);

aclval_spec_t ae2f_ccpure aclval_get_spec(
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_idx_specconst
		);

aclval_spec_t aclval_cfg_spec(
		const u64_least		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLVAL_NBYTE_	c_idx_nbyte,
		enum ACLVAL_TY_	c_idx_ty,
		aclspv_wrd_t		c_idx_specconst
		);

aclspv_id_t aclval_cfg_bool(
		const aclspv_bool_t c_val,
		h_aclspv_ctx_t h_ctx
		);

#if	1
#define	aclutil_mk_constant_val_id(c_val, h_ctx)	aclval_cfg(c_val, h_ctx, ACLVAL_NBYTE_4, ACLVAL_TY_SIGNED)
#define	aclutil_mk_constant_val64_id(c_val, h_ctx)	aclval_cfg(c_val, h_ctx, ACLVAL_NBYTE_8, ACLVAL_TY_SIGNED)
#endif

/** 
 * try assigning c_storage_class to c_descriptor_set | c_bind_idx.
 * if storage class is not reserved in that case value does not change.
 *
 * returns current value assigned in c_discriptor_set | c_bind_idx.
 * */
aclspv_wrd_t aclval_cfg_bindidx(
		aclspv_wrd_t c_descriptor_set,
		aclspv_wrd_t c_bind_idx,
		aclspv_wrd_t c_storage_class,
		h_aclspv_ctx_t h_ctx);

aclspv_id_t aclval_cfg_f32(
		const float c_val, 
		h_aclspv_ctx_t h_ctx
		);


aclspv_id_t aclval_cfg_f64(
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

struct aclutil_mk_constant_repvec_t {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_literal_mask;
};

/** @brief repeating vector */
struct aclutil_mk_constant_repvec_t	aclutil_mk_constant_repvec_id(const aclspv_id_t	c_id_ty, const aclspv_id_t c_val, const aclspv_wrd_t c_literal_mask, h_aclspv_ctx_t h_ctx);


ae2f_unused ae2f_inline_f static aclspv_id_t	aclval_cfg_by_enum(const u64_least c_val, h_aclspv_ctx_t h_ctx, enum ACLSPV_ID_ c_ty) {
	switch((uint_leastenum_t)c_ty) {
#define	CASE_NUM(k, nbyte, ty)	case (k): return	\
		aclval_cfg(c_val, h_ctx, (nbyte), (ty))

		CASE_NUM(ACLSPV_ID_I8, ACLVAL_NBYTE_1, ACLVAL_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I16, ACLVAL_NBYTE_2, ACLVAL_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I32, ACLVAL_NBYTE_4, ACLVAL_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I64, ACLVAL_NBYTE_8, ACLVAL_TY_SIGNED);

		CASE_NUM(ACLSPV_ID_U8, ACLVAL_NBYTE_1, ACLVAL_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U16, ACLVAL_NBYTE_2, ACLVAL_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U32, ACLVAL_NBYTE_4, ACLVAL_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U64, ACLVAL_NBYTE_8, ACLVAL_TY_UNSIGNED);

		CASE_NUM(ACLSPV_ID_F16, ACLVAL_NBYTE_2, ACLVAL_TY_FLT);
		CASE_NUM(ACLSPV_ID_F32, ACLVAL_NBYTE_4, ACLVAL_TY_FLT);
		CASE_NUM(ACLSPV_ID_F64, ACLVAL_NBYTE_8, ACLVAL_TY_FLT);

#undef	CASE_NUM

		case ACLSPV_ID_BOOL: return aclval_cfg_bool((aclspv_bool_t)c_val, h_ctx);

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
		aclval_cfg_spec(c_val, h_ctx, (nbyte), (ty), c_key).m_val

		CASE_NUM(ACLSPV_ID_I8, ACLVAL_NBYTE_1, ACLVAL_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I16, ACLVAL_NBYTE_2, ACLVAL_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I32, ACLVAL_NBYTE_4, ACLVAL_TY_SIGNED);
		CASE_NUM(ACLSPV_ID_I64, ACLVAL_NBYTE_8, ACLVAL_TY_SIGNED);

		CASE_NUM(ACLSPV_ID_U8, ACLVAL_NBYTE_1, ACLVAL_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U16, ACLVAL_NBYTE_2, ACLVAL_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U32, ACLVAL_NBYTE_4, ACLVAL_TY_UNSIGNED);
		CASE_NUM(ACLSPV_ID_U64, ACLVAL_NBYTE_8, ACLVAL_TY_UNSIGNED);

		CASE_NUM(ACLSPV_ID_F16, ACLVAL_NBYTE_2, ACLVAL_TY_FLT);
		CASE_NUM(ACLSPV_ID_F32, ACLVAL_NBYTE_4, ACLVAL_TY_FLT);
		CASE_NUM(ACLSPV_ID_F64, ACLVAL_NBYTE_8, ACLVAL_TY_FLT);

#undef	CASE_NUM
		default:
			return 0;
	}
}

aclval_t*  aclval_cfg_node2(
		const aclspv_wrd_t	c_key_0,
		const aclspv_wrd_t	c_key_1,
		h_aclspv_ctx_t		h_ctx
		);

ae2f_inline_f static aclval_t* aclval_cfg_node(
		const aclspv_wrd2_t	c_key,
		h_aclspv_ctx_t		h_ctx
		) {
	return aclval_cfg_node2(
			(aclspv_wrd_t)(c_key & 0xFFFFFFFF)
			, (aclspv_wrd_t)(c_key >> 32) & 0xFFFFFFFFF
			, h_ctx
			);
}

#endif
