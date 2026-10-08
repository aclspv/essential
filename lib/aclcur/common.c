#include "./common.h"
#include "./util.h"
#include "./lbl.h"

#include <aclmem.h>

aclspv_id_t	aclcur_cfg_node2(aclspv_id_t c_newid, h_aclspv_ctx_t h_ctx, aclspv_bool_t c) {
	aclspv_wrd_t i;

	ae2f_ifezerr(h_ctx)
		return 0;

	ae2f_ifezerr(s_get_buf(h_ctx))
		return 0;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		aclcur_t* const ae2f_restrict p = &s_get_buf(h_ctx)[i];
		if(p->m_class == ACLCUR_RESERVED)
			return p->m_id;
	}

	ae2f_ifnezerr(s_aclmem_cgrow2(
				&h_ctx->m_cursors
				, ++h_ctx->m_num_cursor
				, sizeof(aclcur_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_CURSOR
				)) return 0;

	{
		const h_aclcur_t p = &s_get_buf(h_ctx)[h_ctx->m_num_cursor - 1];
		aclmemzero(p, sizeof(*p));

		if(h_ctx->m_num_cursor > 1) {
			const h_aclcur_t p_prv = &p[-1];
			p->m_depth = p_prv->m_depth;	/** true on most cases */
		}

		/** skip? */
		if(h_ctx->m_num_cursor > 1) {
			h_aclcur_t p1 = &p[-1];
			p->m_num_skip_prv = c ? p1->m_num_skip_prv + 1 : 0;
			p1 = &p1[-p1->m_num_skip_prv - 1];
			for(; p1 < p; ++p1) {
				p1->m_num_skip += c;
			}
		}
	}

	{
		h_aclcur_t p_lbl_cur	= aclcur_get_lbl_last1(h_ctx);
		h_aclcur_t p = &s_get_buf(h_ctx)[h_ctx->m_num_cursor - 1];

		ae2f_ifezerr(p_lbl_cur) return 0;

		if(p != p_lbl_cur) {
			++p_lbl_cur->m_d.m_lbl.m_nop;
			p->m_lbl = p_lbl_cur->m_id;
		}

		return p->m_id = c_newid ? c_newid : h_ctx->m_id++;
	}

	ae2f_unreachable();
}

ae2f_ccpure aclcur_t*	aclcur_get_node(aclspv_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx) {
	aclspv_wrd_t i = h_ctx->m_num_cursor;

	unless(c) return 0;

	ae2f_ifezerr(h_ctx)
		return 0;

	ae2f_ifezerr(s_get_buf(h_ctx))
		return 0;

	while(i --> 0) {
		aclcur_t* const ae2f_restrict p = &s_get_buf(h_ctx)[i];
		if(p->m_id == c)
			return p;
	}

	return 0;
}

aclcur_t* aclcur_get_node_prv(aclspv_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx) {
	const h_aclcur_t p = aclcur_get_node(c, h_ctx);
	const aclspv_wrd_t pad = (aclspv_wrd_t)(p - s_get_buf(h_ctx));
	const aclspv_wrd_t prvidx = ae2f_expected(p) ? pad - 1 - p->m_num_skip_prv : 0xFFFFFFFF;
	return ae2f_expected_strict(prvidx < h_ctx->m_num_cursor) ? &s_get_buf(h_ctx)[prvidx] : 0;
}

aclcur_t* aclcur_get_node_nxt(aclspv_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx) {
	const h_aclcur_t p = aclcur_get_node(c, h_ctx);
	const aclspv_wrd_t pad = (aclspv_wrd_t)(p - s_get_buf(h_ctx));
	const aclspv_wrd_t nxtidx = ae2f_expected(p) ? pad + 1 + p->m_num_skip : 0xFFFFFFFF;
	return ae2f_expected_strict(nxtidx < h_ctx->m_num_cursor) ? &s_get_buf(h_ctx)[nxtidx] : 0;
}


CXCursor aclcur_set_cxcursor(aclspv_id_t c, const CXCursor c_cur, h_aclspv_ctx_t h_ctx) {
	aclcur_t* ae2f_restrict const p = aclcur_get_node(c, h_ctx);
	ae2f_ifezerr(p) return clang_getNullCursor();

	return clang_Cursor_isNull(c_cur)
		? p->m_cur
		: (p->m_cur = c_cur);
}
