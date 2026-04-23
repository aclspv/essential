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

/**
 * @typedef	aclspv_wrd_t
 * @brief	integer as word.
 * 		could be defined with `#define`
 * */
#ifndef	aclspv_wrd_t
#define	aclspv_wrd_t	aclspv_wrd_t
typedef uint_least32_t	aclspv_wrd_t;
#endif

/**
 * @typedef	aclspv_wrdcount_t
 * @brief	the integer type represents the number of word.
 * */
typedef	aclspv_wrd_t	aclspv_wrdcount_t;
extern int ACLSPV_WRD_CHECK[sizeof(aclspv_wrd_t) >= 4 ? 1 : -1];

#include <clang-c/Index.h>

typedef enum ACLSPV_COMPILE_ {
	ACLSPV_COMPILE_OK,
	ACLSPV_COMPILE_ALLOC_FAILED,
	ACLSPV_COMPILE_ALLOC_FAILED_CURSOR,
	ACLSPV_COMPILE_ALLOC_FAILED_CFG,
	ACLSPV_COMPILE_ALLOC_FAILED_EXPR,
	ACLSPV_COMPILE_ALLOC_FAILED_DECLGLOBOBJ,
	ACLSPV_COMPILE_MET_INVAL,
	ACLSPV_COMPILE_ERR_CLANG,
	ACLSPV_COMPILE_TOO_BIG,
	ACLSPV_COMPILE_NO_IMPL,
	ACLSPV_COMPILE_WORKGROUP_MUST_BE_CONSTANT_ARRAY,
	ACLSPV_COMPILE_CXTP_ZERO,
	ACLSPV_COMPILE_STORAGE_CLASS_UNDEFINED,
	ACLSPV_COMPILE_BTCAST_NON_NUMBER,
	ACLSPV_COMPILE_ASSIGN_ON_NON_LVAL,
	ACLSPV_COMPILE_CURSOR_NOT_FOUND,
	ACLSPV_COMPILE_IF_COND_INVAL,

	/** @brief somehow it is considered error but error flag was `OK`. */
	ACLSPV_COMPILE_NOT_GREPPED_BODY
} e_aclspv_compile_t;

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

#endif
