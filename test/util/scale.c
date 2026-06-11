#include "../../lib/util/scale.h"
#include <.aclspv/vec.h>

#include <assert.h>

typedef struct {
	char* m_name;
	char* m_brief;
} evknd;

int main(void) {
	x_aclspv_vec	alloc;
	_aclspv_init_vec(alloc);
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

	_aclspv_stop_vec(_aclspv_free, alloc);

	return 0;
}
