/** @file is_kernel.h @brief find if function is entry point or not */

#ifndef util_is_kernel_h
#define util_is_kernel_h

#include <ae2f/Keys.h>
#include <aclattr/execmodel.h>
#include <aclspv.h>


ae2f_inline_f static aclspv_wrd_t s_aclutil_is_kernel(const CXCursor h_fndecl) {
	enum SpvExecutionModel_ MODEL = SpvExecutionModelMax;
	clang_visitChildren(h_fndecl, s_attr_execmodel, &MODEL);
	return (aclspv_wrd_t)(MODEL ^ SpvExecutionModelMax);
}

#endif
