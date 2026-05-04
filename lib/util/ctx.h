/** @file ctx.h @brief the context of a compiler (something like a state machine) */

#ifndef lib_build_ctx_h
#define lib_build_ctx_h

#include <.aclspv/vec.h>

#include <aclspv.h>

#define	_free(a, b)	free(a)


typedef h_aclspv_ctx_t h_util_ctx_t;

#define	_malloc_with_zero(a)	calloc(a, 1)

#endif
