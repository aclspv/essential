#include "./body.h"
#include <aclctx/err.h>
#include <aclutil/emitx.h>
#include <aclemit/body.h>
#include <aclemit/root.h>
#include <aclattr/main.h>
#include <aclcur/common.h>
#include <aclcur/ssa.h>
#include <aclcur/lbl/pure.h>

enum CXChildVisitResult	aclemit_fndecl_bodycfg(
		CXCursor h_cur,
		h_aclspv_ctx_t	h_ctx,
		aclspv_bool_t* ae2f_restrict	wr_is_const,
		aclspv_bool_t* ae2f_restrict	wr_is_pure
		) {
	aclemit_body_t entp_body;
	entp_body.m_ctx = h_ctx;
	entp_body.m_vid = 0;

	h_ctx->m_count.m_fnimpl = 0;
	h_ctx->m_work_nxt = aclemit_body;
	h_ctx->m_work_data = &entp_body;

	/** statement expressions are carved on demand from inside expression
	 * evaluation, where the body visitor cannot be reached directly. */
	h_ctx->m_stmtexpr = aclemit_body_stmtexpr;

	clang_visitChildren(
			h_cur
			, s_aclattr_main
			, h_ctx);

	aclctx_err_check(h_ctx) {
		return CXChildVisit_Break;
	}

	/** post-carve, pre-emit: complete the loop-header phi chains with
	 * the final-graph back-edge carries (the back edge is not part of
	 * the graph while the header's reads resolve). */
	aclctx_err_check2(h_ctx, aclcur_ssa_loop_carry_fill(h_ctx))
		return CXChildVisit_Break;

	/** the cursor graph is final here: carve is done and aclcur_emit has
	 * not mutated the reference counts yet. capture the function-level
	 * purity now, because the arena is reset at FunctionEnd below, so this
	 * is the only point where the whole function is inspectable. */
	aclctx_err_check2(h_ctx, aclcur_fn_cfg_pure(h_ctx))
		return CXChildVisit_Break;

	if(wr_is_const)	*wr_is_const	= aclcur_fn_is_const(h_ctx);
	if(wr_is_pure)	*wr_is_pure	= aclcur_fn_is_pure(h_ctx);

	/** flush the cursor graph into m_fnimpl: labels, branches, ops, and
	 * register storage (OpVariable) via the register interfaces. */
	aclctx_err_check2(h_ctx, aclcur_emit(h_ctx))
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
		/** falling off the end of a void function is a plain return.
		 * a non-void function that reaches here returned on no known
		 * path (its merge block is dead); plain OpReturn is illegal
		 * there, and C calls the fall-off undefined in any case. */
		const aclspv_wrd_t c_op
			= aclcur_get_retty(h_ctx) == ACLSPV_ID_VOID
			? SpvOpReturn
			: SpvOpUnreachable;

		ae2f_ifezerr(h_ctx->m_count.m_fndef = emit_opcode(
					&h_ctx->m_section.m_fndef
					, h_ctx->m_count.m_fndef
					, c_op, 0
					)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			return CXChildVisit_Break;
		}
	}

	ae2f_ifezerr(h_ctx->m_count.m_fndef = emit_opcode(
				&h_ctx->m_section.m_fndef
				, h_ctx->m_count.m_fndef
				, SpvOpFunctionEnd, 0
				)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
		return CXChildVisit_Break;
	}


	h_ctx->m_num_cursor = 0;

	/** the body is out: a later expression evaluation is not allowed to
	 * carve statements into the finished function. */
	h_ctx->m_stmtexpr	= 0;

	h_ctx->m_work_data	= h_ctx;
	h_ctx->m_work_nxt	= aclemit_root;
	return CXChildVisit_Continue;
}
