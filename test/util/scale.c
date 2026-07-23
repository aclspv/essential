#include "../../lib/util/scale.h"

#include <assert.h>
#include <string.h>

typedef struct {
	char* m_name;
	char* m_brief;
} evknd;

int main(void) {
	x_aclspv_ctx	ctx;
	x_aclspv_vec	alloc;

	aclspv_usrcfg_t	usrcfg;
	memset(&usrcfg, 0, sizeof(usrcfg));

	aclspv_init_ctx(&ctx, &usrcfg);
	aclmem_init(&alloc, &ctx);
	assert(init_scale(&alloc, 0));

	assert(mk_scale_from_vec(&alloc, 0));

#if	1
	assert(mk_scale_from_vec(&alloc, 0));
	assert(mk_scale_from_vec(&alloc, 0));
	assert(mk_scale_from_vec(&alloc, 0));
	assert(mk_scale_from_vec(&alloc, 0));
#endif

	assert(get_scale_from_vec(&alloc, 0));
	assert(get_scale_from_vec(&alloc, 1));
	assert(get_scale_from_vec(&alloc, 2));
	assert(get_scale_from_vec(&alloc, 3));

	aclmem_fini(&alloc);
	aclspv_fini_ctx(&ctx);
	return 0;
}
