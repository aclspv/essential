#ifndef	aclty_default_h
#define aclty_default_h

#include <aclty/common.h>

/** see ACLSPV_ID_ */
typedef aclty_id_t aclty_default_t;
ae2f_ccconst aclspv_bool_t aclty_is_default(const aclty_default_t c);
aclty_default_t aclty_mk_default(const aclty_default_t c, h_aclspv_ctx_t h_ctx);

#endif
