#include "./mallocfree.h"

#if	DBG
#include <stdio.h>
#include <util/log.h>
#include <stdlib.h>
#include <string.h>

ae2f_retnew1(_aclspv_free) void*	aclspv_malloc(size_t c_sz) {
	void* R = calloc(c_sz, 1);
	dbg_prefix(a);
	fprintf(stderr
			, "_aclspv_malloc\n"
			"\tsize_t c_sz = %lu\n"
			"\treturn: %p\n"
			, (unsigned long)c_sz
			, R
			);
	fflush(stderr);
	return R;
}

void  aclspv_free(void* c_ptr, const size_t c_sz) {
	dbg_prefix(a);
	fprintf(stderr
			, "_aclspv_free\n"
			"\tvoid* c_ptr = %p\n"
			"\tsize_t c_sz = %lu\n"

			, (void*)c_ptr, (const size_t)c_sz
			);
	fflush(stderr);
	free(c_ptr);
}


void aclspv_memcpy(void* ae2f_restrict const wr_dst, const void* ae2f_restrict const rd_src, const size_t c_btsize) {
	dbg_prefix();
	fprintf(stderr
			, "_aclspv_memcpy\n"
			"\twr_dst: %p\n"
			"\trd_src: %p\n"
			"\tc_btsize: %lu\n"
			, wr_dst, rd_src, (unsigned long)c_btsize);

	memcpy(wr_dst, rd_src, c_btsize);
	return;
}

#endif
