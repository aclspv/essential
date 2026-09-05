#ifndef aclutil_cxtp_h
#define	aclutil_cxtp_h

#include <ae2f/Keys.h>

#include "./aclty/arr.h"
#include "./aclty/vec.h"
#include "./aclty/ptr.h"
#include "./aclty/struct.h"
#include "./aclty/fn.h"
#include "./aclty/by_type.h"
#include "./aclty/by_cursor.h"

ae2f_ccpure	aclty_t*	aclty_find(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);
aclconst_specsz			aclty_getsize(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);
ae2f_ccpure	aclspv_wrd_t	aclty_getalign(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);

#endif
