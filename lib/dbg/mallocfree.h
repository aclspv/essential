#ifndef	acldbg_mallocfree_h
#define	acldbg_mallocfree_h


#include <stddef.h>
#include <ae2f/cc/fnpretty.h>
#include <ae2f/cc/qualifier.h>
#include <ae2f/cc/inline.h>

void  aclspv_free(void* c_ptr, const size_t c_sz);
void* aclspv_malloc(size_t c_sz);
void aclspv_memcpy(void* ae2f_restrict const wr_dst, const void* ae2f_restrict const rd_src, const size_t c_btsize);

#define	_aclspv_free	aclspv_free
#define	_aclspv_malloc	aclspv_malloc
#define	_aclspv_memcpy	aclspv_memcpy


#endif

