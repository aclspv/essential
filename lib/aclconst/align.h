#ifndef	aclconst_align_h
#define aclconst_align_h

#include "./specsize.h"

aclconst_specsz	aclconst_align_from_size(
		aclconst_specsz	c,
		h_aclspv_ctx_t	h_ctx
		);

aclconst_specsz	aclconst_align_from_size2(
		aclspv_wrd_t	c,
		h_aclspv_ctx_t	h_ctx
		);

#endif
