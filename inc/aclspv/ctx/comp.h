#ifndef	aclspv_ctx_comp_h
#define aclspv_ctx_comp_h

#include "./common.h"
#include <aclspv/wrd.h>

#include <clang-c/Index.h>

#include <aclspv/enum/err.h>
#include <aclspv/link.h>
#include <ae2f/Cxx/extern.h>

/** 
 * @brief	reads `c_cur_root` and merge it to `h_ctx`
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile(
		const h_aclspv_ctx_t	h_ctx,
		CXCursor		c_cur_root
		);

/** 
 * @brief	reads `h_tu` and merge it to `h_ctx`.
 * */
ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile1(
		const h_aclspv_ctx_t	h_ctx,
		CXTranslationUnit	h_tu
		);

ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile2(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt,
		CXIndex							h_cxidx
		);

ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_ctx_compile3(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
		);

#endif
