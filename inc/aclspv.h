/**
 * @file aclspv.h
 * Compiles OpenCL C kernel code to SPIR-V for Vulkan.
 * */

#ifndef aclspv_h
#define aclspv_h

#include <ae2f/cc/profiles.h>

#include "./aclspv3rd/spirv-unified1.h"
#include "./aclspv3rd/clang/idx.h"

#include <ae2f/Keys.h>
#include <ae2f/.Call.h>

#include <ae2f/Cxx/Nil.h>
#include <ae2f/cc/dylink.h>

/**
 * @def ACLSPV_ABI_IMPL	
 * @brief Implementation as ABI
 *
 * @def ACLSPV_ABI_DECL	
 * @brief Declaration as ABI
 * */

#ifndef	ACLSPVLIB
#	define	ACLSPVLIB	0
#endif

#if	ae2f_IS_SHARED
#	if ACLSPVLIB
#		define ACLSPV_ABI_LINK	ae2f_impl
#	else
#		define ACLSPV_ABI_LINK	ae2f_decl
#	endif
#else
#	define	ACLSPV_ABI_LINK
#endif

#define	ACLSPV_ABI_IMPL	
#define	ACLSPV_ABI_DECL	ACLSPV_ABI_LINK

#include <ae2f/c90/stdint.h>
#include <ae2f/c90/stdint/enum.h>
#include <stddef.h>

/**
 * @typedef	aclspv_wrd_t
 * @brief	integer as word.
 * 		could be defined with `#define`
 * */
#ifndef	aclspv_wrd_t
#define	aclspv_wrd_t	uint_least32_t
#endif

#define	aclspv_ver_spv(c_mjr, c_mnr)	\
	((ae2f_static_cast(aclspv_wrd_t, c_mjr) << ae2f_static_cast(aclspv_wrd_t, 16))	|	\
	 (ae2f_static_cast(aclspv_wrd_t, c_mnr) << ae2f_static_cast(aclspv_wrd_t, 8)))

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


typedef enum ACLSPV_COMPILE_ {
	ACLSPV_COMPILE_OK,
	ACLSPV_COMPILE_PRM_INVAL_NIL,

	ACLSPV_COMPILE_NOIMPL,
	ACLSPV_COMPILE_NOIMPL_V_WRKGROUP_SIZE,
	ACLSPV_COMPILE_NOIMPL_CAPABILITY,
	ACLSPV_COMPILE_NOIMPL_EXTENSION,

	ACLSPV_COMPILE_ALLOC_FAILED,
	ACLSPV_COMPILE_ALLOC_FAILED_CURSOR,
	ACLSPV_COMPILE_ALLOC_FAILED_CFG,
	ACLSPV_COMPILE_ALLOC_FAILED_EXPR,
	ACLSPV_COMPILE_ALLOC_FAILED_DECLGLOBOBJ,
	ACLSPV_COMPILE_ALLOC_FAILED_VEC,
	ACLSPV_COMPILE_ALLOC_FAILED_ID,
	ACLSPV_COMPILE_ALLOC_FAILED_ASM,
	ACLSPV_COMPILE_ALLOC_FAILED_CAPABILITY,
	ACLSPV_COMPILE_ALLOC_FAILED_CXTP,

	ACLSPV_COMPILE_MET_INVAL,
	ACLSPV_COMPILE_MET_INVAL_BRK,
	ACLSPV_COMPILE_MET_INVAL_CONTINUE,
	ACLSPV_COMPILE_MET_INVAL_UNREACHABLE,

	ACLSPV_COMPILE_ERR_CLANG,
	ACLSPV_COMPILE_ERR_CXIDX,
	ACLSPV_COMPILE_ERR_CXTU,

	ACLSPV_COMPILE_TOO_BIG,
	ACLSPV_COMPILE_TOO_BIG_EXPRVISIT,

	ACLSPV_COMPILE_WORKGROUP_MUST_BE_CONSTANT_ARRAY,
	ACLSPV_COMPILE_WORKGROUP_SIZE_NO_INT,
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
	ACLSPV_COMPILE_UNHANDLED_VISIT,
	ACLSPV_COMPILE_UNHANDLED_CARVE,

	ACLSPV_COMPILE_DIAG_FTAL,
	ACLSPV_COMPILE_DIAG_ERR
} e_aclspv_compile_t;

typedef struct {
	void* ae2f_restrict 	m_p;
	size_t	m_sz;
} x_aclspv_vec;

#define	__ENUM()			\
	__X(SYMBOL),			\
	__X(BUF64),			\
	__X(OGL),			\
	__X(LOGICAL),			\
	__X(DUMP_DIAGNOSTICS),		\
	__X(KILL_ON_DIAGNOSTICS),	\
	__X(EXPLICIT_CAPABILITY),	\
	__X(EXPLICIT_EXTENSION)

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
enum ACLSPV_ID_ {
	ACLSPV_ID_VOID = 1, 
	ACLSPV_ID_BOOL,
	ACLSPV_ID_I8,
	ACLSPV_ID_U8,
	ACLSPV_ID_I16,
	ACLSPV_ID_U16,
	ACLSPV_ID_I32,
	ACLSPV_ID_U32,
	ACLSPV_ID_I64,
	ACLSPV_ID_U64,
	ACLSPV_ID_F16,
	ACLSPV_ID_F32,
	ACLSPV_ID_F64,
	ACLSPV_ID_END
};

typedef struct a_aclspv_ctx* ae2f_restrict h_aclspv_ctx_t;


typedef struct a_aclspv_ctx {
	/** @brief see `ACLSPV_CTX_FLAG0_` */
	uint_leastenum_t	m_flag0;

	e_aclspv_compile_t	m_err;

	/**
	 * @brief
	 * A output bitcode(spir-v)'s version.
	 * Use `aclspv_ver_spv` to specify it
	 * */
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
	aclspv_wrdcount_t	m_num_constant_cache;

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

		/**
		 * [emit::body]	expression stack
		 * */
		x_aclspv_vec	m_v3;
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

	aclspv_id_t		m_id_defaults[ACLSPV_ID_END];
} x_aclspv_ctx;


/**
 * @brief	initialisse `i_ctx` with `c_init_flag0` and `c_spirv_version`.
 *		`i_ctx` must be released by `aclspv_fini_ctx`.
 * */
ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_wrd_t	c_init_size_exp,
		const aclspv_wrd_t	c_init_flag0,
		const aclspv_wrd_t	c_spirv_version
		);

/** 
 * @brief	reads `c_cur_root` and merge it to `h_ctx`
 * */
ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile(
		const h_aclspv_ctx_t	h_ctx,
		CXCursor		c_cur_root
		);

/** 
 * @brief	reads `h_tu` and merge it to `h_ctx`.
 * */
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

/** 
 * @brief	free the resources of `d_ctx`.
 * */
ae2f_extern ACLSPV_ABI_DECL void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		);

/**
 * @brief	dump the spir-v output from `h_ctx` to `ir_output_opt`, with length to `r_output_count_opt`.
 * 		value of `ir_output_opt` must be manually freed by `free`.
 * */
ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_dump_ctx_output(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrdcount_t* ae2f_restrict const		r_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict	ir_output_opt
		);

/**
 * @brief	dump the spir-v output from `h_ctx` to `ret`.
 * @details	word count of `ret` is considered as a value of `rdwr_count_ret`.
 * 		when word count is not sufficient, value of `rdwr_count_ret` will be changed to 
 * 			actual required word count of `ret`.
 * */
ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_dump_ctx_output1(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const	rdwr_count_ret,
		aclspv_wrd_t* ae2f_restrict const	ret
		);

/**
 * @brief	check if capacity `c_cap` is included in `rd_ctx`.
 * */
ae2f_extern ACLSPV_ABI_DECL ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_chck_capability_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const enum SpvCapability_		c_cap
		);

/**
 * @brief	add a capacity `c_cap` to `h_ctx`
 * */
ae2f_extern ACLSPV_ABI_DECL ae2f_noexcept aclspv_bool_t aclspv_add_capability_from_ctx(
		h_aclspv_ctx_t				h_ctx,
		const enum SpvCapability_		c_cap
		);

ae2f_extern ACLSPV_ABI_DECL ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_chck_extension_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const char* ae2f_restrict const		c_ext
		);

ae2f_extern ACLSPV_ABI_DECL ae2f_noexcept aclspv_bool_t aclspv_add_extension_from_ctx(
		const h_aclspv_ctx_t			rd_ctx,
		const char* ae2f_restrict const		c_ext
		);

#endif
