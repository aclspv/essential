#include "./mallocfree.h"

#include <stdio.h>
#include <util/log.h>
#include <stdlib.h>
#include <string.h>
#include <ae2f/cc/fnpretty.h>

#if	DBG
static	unsigned s_malloc_count = 0;
#endif

ae2f_retnew1(aclspv_free) void*	aclspv_malloc(size_t c_sz) {
	void* R = calloc(1, c_sz);

#ifndef	DBG
	if(R) ++s_malloc_count;
#endif

	dbg_prefix(a);
	dbg_call((void), fprintf(stderr
			, "_aclspv_malloc\n"
			"\tsize_t c_sz = %lu\n"
			"\treturn: %p\n"
			"\tstack: %u\n"
			, (unsigned long)c_sz
			, R
			, s_malloc_count
			));
	return R;
}

void  aclspv_free(void* c_ptr, const ae2f_unused size_t c_sz) {
	dbg_prefix(a);

#ifndef	DBG
	if(c_ptr) --s_malloc_count;
#endif

	dbg_call((void), fprintf(stderr
				, "_aclspv_free\n"
				"\tvoid* c_ptr = %p\n"
				"\tsize_t c_sz = %lu\n"
				"\tstack: %u\n"
				, (void*)c_ptr
				, (const unsigned long)c_sz
				, s_malloc_count
				));
	free(c_ptr);
}


void aclspv_memcpy(void* ae2f_restrict const wr_dst, const void* ae2f_restrict const rd_src, const size_t c_btsize) {
	dbg_prefix();
	dbg_call((void), fprintf(stderr
			, "_aclspv_memcpy\n"
			"\twr_dst: %p\n"
			"\trd_src: %p\n"
			"\tc_btsize: %lu\n"
			, wr_dst
			, rd_src
			, (unsigned long)c_btsize));

	memcpy(wr_dst, rd_src, c_btsize);
	return;
}


