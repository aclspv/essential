#include "./emit.h"
#include "../common.h"
#include "../util.h"
#include <ae2f/cc/assume.h>
#include <util/wrdemit.h>

#if	__ACLSPV_CHECK_EXTRA
#define	s_assume(c, c_err) ae2f_ifezerr(c) { err_puts("assumed failed: " #c); return (c_err); }
#else
#define	s_assume(c, c_err)	ae2f_assume(c)
#endif

aclspv_compile_t EMIT_aclcur_opx(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {

	aclspv_wrd_t	i = 0;
	aclspv_wrd_t	idx;
	aclspv_wrd_t* ae2f_restrict wr;
	const aclspv_wrd_t	ANCHOUR = h_ctx->m_count.m_fnimpl;

	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class == ACLCUR_OPX, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_val == h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE), ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	unless(h->m_num_ref) return ACLSPV_COMPILE_OK;

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emit_wrd(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl + h->m_d.m_opx.m_num + 2
				, 0
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	wr = &aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fnimpl)[ANCHOUR];
	wr[0] = aclspv_opcode_to_wrd(h->m_d.m_opx.m_op, h->m_d.m_opx.m_num + 2);
	wr[1] = h->m_ty;
	wr[2] = h->m_id;

	idx = h->m_d.m_opx.m_o_head;
	while(idx) {
		const h_aclcur_t p_mbr = &s_get_buf(h_ctx)[idx];
#if __ACLSPV_CHECK_EXTRA
		ae2f_ifnezerr_strict(p_mbr->m_class != ACLCUR_OPX_MBR) return ACLSPV_COMPILE_UNHANDLED;
#endif
		wr[3 + i] = p_mbr->m_d.m_opx_mbr.m_operand;
		idx = p_mbr->m_d.m_opx_mbr.m_nxt;
		++i;
	}

#if __ACLSPV_CHECK_EXTRA
	ae2f_ifnezerr_strict(i != h->m_d.m_opx.m_num) return ACLSPV_COMPILE_UNHANDLED;
#endif

	return ACLSPV_COMPILE_OK;
}

