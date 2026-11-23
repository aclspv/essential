#include "./emit.h"
#include "../common.h"
#include "../util.h"

#include <aclutil/emitx.h>
#include <aclutil/wrdemit.h>

#include <util/log.h>

aclspv_compile_t EMIT_aclcur_sw(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t	n_case;
	aclspv_wrd_t	idx_case;
	aclspv_wrd_t	i;
	aclspv_wrd_t	wc;
	aclspv_wrd_t	anchor;
	aclspv_wrd_t* ae2f_restrict wr;

	ae2f_assume(h);
	ae2f_assume(h->m_class == ACLCUR_SW);
	ae2f_assume(h_ctx);

	if(h->m_d.m_sw.m_num_words != 1 && h->m_d.m_sw.m_num_words != 2) {
		err_puts("[aclcur::sw::emit] bad literal width");
		return ACLSPV_COMPILE_UNHANDLED;
	}

	/** selection merge must immediately precede the switch. */
	if(h->m_d.m_sw.m_merge) {
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_3(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpSelectionMerge
					, h->m_d.m_sw.m_merge
					, h->m_d.m_sw.m_ctl
					))
			return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
	}

	n_case	= h->m_d.m_sw.m_num_case;
	idx_case = h->m_d.m_sw.m_cases_head;

	/** word count: opcode + selector + default + n * (literal + label). */
	if(n_case > ((aclspv_wrd_t)0xFFFFFFFFu - 3u) / (h->m_d.m_sw.m_num_words + 1u)) {
		err_puts("[aclcur::sw::emit] switch too large");
		return ACLSPV_COMPILE_TOO_BIG;
	}
	wc = 3u + n_case * (h->m_d.m_sw.m_num_words + 1u);

	anchor = h_ctx->m_count.m_fnimpl;
	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emit_wrd(
				&h_ctx->m_section.m_fnimpl
				, anchor + wc - 1u
				, 0
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	wr = &aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fnimpl)[anchor];
	wr[0] = aclspv_opcode_to_wrd(SpvOpSwitch, wc - 1u);
	wr[1] = h->m_d.m_sw.m_sel;
	wr[2] = h->m_d.m_sw.m_default;

	i = 3;

	while(idx_case) {
		h_aclcur_t p_case = aclcur_get_node_idx(idx_case, h_ctx);

		ae2f_ifezerr(p_case)
			return ACLSPV_COMPILE_UNHANDLED;
		ae2f_ifnezerr_strict(p_case->m_class != ACLCUR_SW_CASE)
			return ACLSPV_COMPILE_UNHANDLED;

		wr[i++] = p_case->m_d.m_sw_case.m_val_lo;

		if(h->m_d.m_sw.m_num_words == 2)
			wr[i++] = p_case->m_d.m_sw_case.m_val_hi;

		wr[i++] = p_case->m_d.m_sw_case.m_lbl;
		idx_case = p_case->m_d.m_sw_case.m_nxt;
	}

	ae2f_ifnezerr_strict(i != wc) {
		err_puts("[aclcur::sw::emit] case count mismatch");
		return ACLSPV_COMPILE_UNHANDLED;
	}

	return ACLSPV_COMPILE_OK;
}
