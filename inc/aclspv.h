/**
 * @file aclspv.h
 * Compiles OpenCL C kernel code to SPIR-V for Vulkan.
 * */

#ifndef aclspv_h
#define aclspv_h

#include <ae2f/Keys.h>
#include <ae2f/cc.h>
#include <ae2f/.Call.h>

/**
 * @def ACLSPV_ABI_IMPL	
 * @brief Implementation as ABI
 *
 * @def ACLSPV_ABI_DECL	
 * @brief Declaration as ABI
 * */

#if ae2f_IS_SHARED
#define ACLSPV_ABI_DECL ae2f_decl
#define ACLSPV_ABI_IMPL	ae2f_impl
#else
#define ACLSPV_ABI_DECL
#define ACLSPV_ABI_IMPL
#endif

#include <ae2f/c90/StdInt.h>
#include <stddef.h>

/**
 * @typedef	aclspv_wrd_t
 * @brief	integer as word.
 * 		could be defined with `#define`
 * */
#ifndef	aclspv_wrd_t
#define	aclspv_wrd_t	uint_least32_t
#endif

/**
 * @typedef	aclspv_wrdcount_t
 * @brief	the integer type represents the number of word.
 * */
typedef	aclspv_wrd_t	aclspv_wrdcount_t;

/**
 * @typedef	aclpsv_id_t
 * @brief	represents id in spir-v
 * */
typedef	aclspv_wrd_t	aclspv_id_t;
typedef aclspv_wrd_t	aclspv_bool_t;

typedef int ACLSPV_WRD_CHECK[sizeof(aclspv_wrd_t) >= 4 ? 1 : -1];

#include <clang-c/Index.h>

typedef enum ACLSPV_COMPILE_ {
	ACLSPV_COMPILE_OK,
	ACLSPV_COMPILE_PRM_INVAL_NIL,

	ACLSPV_COMPILE_ALLOC_FAILED,
	ACLSPV_COMPILE_ALLOC_FAILED_CURSOR,
	ACLSPV_COMPILE_ALLOC_FAILED_CFG,
	ACLSPV_COMPILE_ALLOC_FAILED_EXPR,
	ACLSPV_COMPILE_ALLOC_FAILED_DECLGLOBOBJ,
	ACLSPV_COMPILE_ALLOC_FAILED_VEC,
	ACLSPV_COMPILE_ALLOC_FAILED_ID,
	ACLSPV_COMPILE_ALLOC_FAILED_ASM,

	ACLSPV_COMPILE_MET_INVAL,

	ACLSPV_COMPILE_ERR_CLANG,
	ACLSPV_COMPILE_ERR_CXIDX,
	ACLSPV_COMPILE_ERR_CXTU,

	ACLSPV_COMPILE_TOO_BIG,
	ACLSPV_COMPILE_NO_IMPL,
	ACLSPV_COMPILE_WORKGROUP_MUST_BE_CONSTANT_ARRAY,
	ACLSPV_COMPILE_CXTP_ZERO,
	ACLSPV_COMPILE_STORAGE_CLASS_UNDEFINED,
	ACLSPV_COMPILE_BTCAST_NON_NUMBER,
	ACLSPV_COMPILE_CURSOR_NOT_FOUND,
	ACLSPV_COMPILE_IF_COND_INVAL,
	ACLSPV_COMPILE_INSUFFICIENT_BUFFER_LENGTH,

	ACLSPV_COMPILE_WHILE_LACK_COMPONENTS,
	ACLSPV_COMPILE_INFINITE_LOOP_FOUND,

	ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD,
	ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW,
	ACLSPV_COMPILE_ARITHMETIC_CAST_ID_IS_UNDEFINED_OLD,

	ACLSPV_COMPILE_ASSIGN_ON_NON_LVAL,
	ACLSPV_COMPILE_ASSIGN_CAST_GOT_ZERO,

	ACLSPV_COMPILE_UNHANDLED,
	ACLSPV_COMPILE_UNHANDLED_BODY,
	ACLSPV_COMPILE_UNHANDLED_WHILE,
	ACLSPV_COMPILE_UNHANDLED_VISIT_SIMPLE,

	ACLSPV_COMPILE_DIAG_FTAL,
	ACLSPV_COMPILE_DIAG_ERR
} e_aclspv_compile_t;

typedef struct {
	void* ae2f_restrict 	m_p;
	size_t	m_sz;
} x_aclspv_vec;

/**
 * @fn	aclspv_compile
 * @brief	compile files and generate spir-v for vulkan
 * @returns	<INIT:aclspv_free_obj>
 * */
ae2f_extern ACLSPV_ABI_DECL e_aclspv_compile_t 
aclspv_compile(
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		aclspv_wrdcount_t* ae2f_restrict			rwr_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict		rwr_output,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
	      );

#define	__ENUM()			\
	__X(SYMBOL),			\
	__X(BUF64),			\
	__X(OGL),			\
	__X(LOGICAL),			\
	__X(DUMP_DIAGNOSTICS),		\
	__X(KILL_ON_DIAGNOSTICS)	

#define	__X(A)	ACLSPV_CTX_FLAGPAD0_##A

enum ACLSPV_CTX_FLAGPAD0_ {
	__ENUM()
};

#undef	__X
#define	__X(A)	ACLSPV_CTX_FLAG0_##A = 1 << ACLSPV_CTX_FLAGPAD0_##A

enum ACLSPV_CTX_FLAG0_ {
	__ENUM()
};

#undef	__X
#undef	__ENUM

/** type id */
typedef enum ID_DEFAULT_ {
	ID_DEFAULT_VOID = 1, 
	ID_DEFAULT_BOOL,
	ID_DEFAULT_I8,
	ID_DEFAULT_U8,
	ID_DEFAULT_I16,
	ID_DEFAULT_U16,
	ID_DEFAULT_I32,
	ID_DEFAULT_U32,
	ID_DEFAULT_I64,
	ID_DEFAULT_U64,
	ID_DEFAULT_F16,
	ID_DEFAULT_F32,
	ID_DEFAULT_F64,
	ID_DEFAULT_END
} e_id_default;

typedef struct a_aclspv_ctx* ae2f_restrict h_aclspv_ctx_t;


typedef struct a_aclspv_ctx {
	/** @brief see `ACLSPV_CTX_FLAG0_` */
	uint_leastenum_t	m_flag0;

	e_aclspv_compile_t	m_err;

	aclspv_wrd_t		m_spirv_ver;

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

	/**
	 * @var		m_id
	 * @brief	id
	 * */
	aclspv_id_t		m_id;

	/** 
	 * @var m_scale_vars 
	 * @see util_bind 
	 * */
	x_aclspv_vec            m_scale_vars;

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

	/** 
	 * @brief
	 * cache for complex types which needs to be stored somewhere
	 * for its use see util/cxtp.h
	 *
	 * [aclutil_cxtp]
	 * */
	x_aclspv_vec		m_type_uniques;

	struct {
		aclspv_wrd_t	m_w0, m_w1, m_w2, m_w3;

		/** 
		 * [emit::body] name
		 * [emit::body] vector initialiser
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

	struct {
		aclspv_wrdcount_t
			m_capability,
			m_ext,
			m_memmodel,
			m_entp,
			m_execmode,
			m_name,
			m_decorate,
			m_types,
			m_vars,
			m_fndef,
			m_fnimpl;
	}			m_count;

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
			m_ext,
			m_memmodel,
			m_entp,
			m_execmode,
			m_name,
			m_decorate,
			m_types,
			m_vars,
			m_fndef,
			m_fnimpl;
	}			m_section;

	aclspv_id_t		m_id_defaults[ID_DEFAULT_END];
} x_aclspv_ctx;

/**
 * @fn	aclspv_compile
 * @brief	compile files and generate spir-v for vulkan
 * @returns	<INIT:aclspv_free_obj>
 * */
ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ 
aclspv_compile(
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		aclspv_wrdcount_t* ae2f_restrict			rwr_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict		rwr_output,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
	      );

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_wrd_t	c_init_size_exp,
		const aclspv_wrd_t	c_init_flag0,
		const aclspv_wrd_t	c_spirv_version
		);

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile(
		const h_aclspv_ctx_t	h_ctx,
		CXCursor		c_cur_root
		);

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile1(
		const h_aclspv_ctx_t	h_ctx,
		CXTranslationUnit	h_tu
		);

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile2(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt,
		CXIndex							h_cxidx
		);

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile3(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
		);

ae2f_extern ACLSPV_ABI_DECL void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		);

ae2f_extern enum ACLSPV_COMPILE_ aclspv_dump_ctx_output(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrdcount_t* ae2f_restrict const		r_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict	r_output_opt
		);

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_dump_ctx_output1(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const	rdwr_count_ret,
		aclspv_wrd_t* ae2f_restrict const	ret
		);


#endif
