#ifndef	aclspv_enum_id_h
#define aclspv_enum_id_h

/** 
 * type id. 
 * enum is ordered by arithmetic cast priority.
 * */
enum ACLSPV_ID_ {
	ACLSPV_ID_VOID = 1, 
	ACLSPV_ID_BOOL,

	ACLSPV_ID_U8,
	ACLSPV_ID_I8,

	ACLSPV_ID_U16,
	ACLSPV_ID_I16,

	ACLSPV_ID_U32,
	ACLSPV_ID_I32,

	ACLSPV_ID_U64,
	ACLSPV_ID_I64,

#if	__ACLSPV_SCLR_F16
	ACLSPV_ID_F16,
#endif
#if	__ACLSPV_SCLR_BF16
	ACLSPV_ID_BF16,
#endif
	ACLSPV_ID_F32,
	ACLSPV_ID_F64,

	ACLSPV_ID_END
};

#endif
