#include "../fn.h"
#include "../implutil.h"
#include <aclmem.h>
#include <aclty.h>

aclutil_cxtp* aclty_cfg_fn(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx
		) {
	struct __ret r;
	aclutil_cxtp t;

	aclmemzero(&t, sizeof(t));
	t.m_class = ACLUTIL_CXTYPE_FUNC;
	t.m_info.m_fn.m_ret_id = c_ret_id;

	r = s_ret_new(h_ctx, &t, 0, 1);
	ae2f_ifezerr(r.m_id)
		return 0;

	return aclty_find(r.m_id, h_ctx);
}
