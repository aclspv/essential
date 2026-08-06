#ifndef aclutil_cxtp_h
#define	aclutil_cxtp_h

#include <ae2f/Keys.h>

#include "./cxtp/arr.h"
#include "./cxtp/vec.h"
#include "./cxtp/ptr.h"
#include "./cxtp/struct.h"
#include "./cxtp/fn.h"
#include "./cxtp/by_type.h"
#include "./cxtp/by_cursor.h"

ae2f_ccpure	aclutil_cxtp*	aclutil_find_cxtp(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);
aclconst_specsz			aclutil_cxtp_sizeof(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);
ae2f_ccpure	aclspv_wrd_t	aclutil_cxtp_alignof(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);

#endif
