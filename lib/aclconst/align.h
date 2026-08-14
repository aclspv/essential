#ifndef	aclconst_align_h
#define aclconst_align_h

#include "./specsize.h"

aclconst_specsz	aclconst_align_from_size(
		aclconst_specsz	c,
		h_aclspv_ctx_t	h_ctx
		);

aclconst_specsz	aclconst_alignup(
		aclconst_specsz	c,
		aclconst_specsz	c_algn,
		h_aclspv_ctx_t	h_ctx
		);

ae2f_inline_f static aclconst_specsz aclconst_alignup1(
		aclconst_specsz	c,
		aclspv_wrd_t	c_algn,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz algn;
	algn.m_id = 0;
	algn.m_lit.m_wrd = c_algn;

	return aclconst_alignup(c, algn, h_ctx);
}

aclconst_specsz	aclconst_align_from_size2(
		aclspv_wrd_t	c,
		h_aclspv_ctx_t	h_ctx
		);

#endif
