#include "./body.h"
#include <aclutil/emitx.h>
#include <aclemit/body.h>
#include <aclemit/root.h>
#include <aclattr/main.h>
#include <aclcur/common.h>
#include <aclcur/ssa.h>

enum CXChildVisitResult	aclemit_fndecl_bodycfg(
		CXCursor h_cur,
		h_aclspv_ctx_t	h_ctx
		) {
	h_ctx->m_count.m_fnimpl = 0;

	h_ctx->m_work_nxt = aclemit_entp_body;
	h_ctx->m_work_data = h_ctx;

	clang_visitChildren(
			h_cur
			, s_aclattr_main
			, h_ctx);

	ae2f_ifnezerr(h_ctx->m_err) {
		return CXChildVisit_Break;
	}

	/** post-carve, pre-emit: complete the loop-header phi chains with
	 * the final-graph back-edge carries (the back edge is not part of
	 * the graph while the header's reads resolve). */
	ae2f_ifnezerr(h_ctx->m_err = aclcur_ssa_loop_carry_fill(h_ctx))
		return CXChildVisit_Break;

	/** flush the cursor graph into m_fnimpl: labels, branches, ops, and
	 * register storage (OpVariable) via the register interfaces. */
	ae2f_ifnezerr(h_ctx->m_err = aclcur_emit(h_ctx))
		return CXChildVisit_Break;

	if(h_ctx->m_count.m_fnimpl) {
		ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&h_ctx->m_section.m_fndef
				, h_ctx->m_count.m_fndef + h_ctx->m_count.m_fnimpl
				, sizeof(aclspv_wrd_t)
				, ACLSPV_COMPILE_ALLOC_FAILED
			       )) return CXChildVisit_Break;

		memcpy(
				aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fndef) + h_ctx->m_count.m_fndef
				, h_ctx->m_section.m_fnimpl.m_p
				, (size_t)aclutil_count_to_sz(h_ctx->m_count.m_fnimpl)
		      );

		h_ctx->m_count.m_fndef += h_ctx->m_count.m_fnimpl;
	}

	unless(aclcur_get_fnret(h_ctx)) {
		ae2f_ifezerr(h_ctx->m_count.m_fndef = emit_opcode(
					&h_ctx->m_section.m_fndef
					, h_ctx->m_count.m_fndef
					, SpvOpReturn, 0
					)) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			return CXChildVisit_Break;
		}
	}

	ae2f_ifezerr(h_ctx->m_count.m_fndef = emit_opcode(
				&h_ctx->m_section.m_fndef
				, h_ctx->m_count.m_fndef
				, SpvOpFunctionEnd, 0
				)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
		return CXChildVisit_Break;
	}


	h_ctx->m_num_cursor = 0;

	h_ctx->m_work_data	= h_ctx;
	h_ctx->m_work_nxt	= aclemit_root;
	return CXChildVisit_Continue;
}
