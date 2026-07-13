#include "./body.h"
#include <util/emitx.h>
#include <util/fnctx.h>
#include <.aclspv/util/fnctx.h>
#include <aclemit/body.h>
#include <aclemit/root.h>
#include <aclattr/main.h>

enum CXChildVisitResult	aclemit_fndecl_bodycfg(
		CXCursor h_cur,
		const aclutil_fnctx* ae2f_restrict rd_fnctx,
		h_aclspv_ctx_t			h_ctx
		) {
	aclutil_fnctx	fnctx = *rd_fnctx;
	fnctx.m_break_loop	= 0;
	fnctx.m_var_stage	= 1;
	fnctx.m_ctx		= h_ctx;

	h_ctx->m_count.m_fnimpl = 0;

	ae2f_ifezerr(h_ctx->m_count.m_fndef = aclutil_emitx_2(
				&h_ctx->m_section.m_fndef
				, h_ctx->m_count.m_fndef
				, SpvOpLabel
				, fnctx.m_branch = h_ctx->m_id++)) {
		fnctx.m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
		return CXChildVisit_Break;
	}

	h_ctx->m_work_nxt = aclemit_entp_body;
	fnctx.m_ctx->m_work_data = &fnctx;

	clang_visitChildren(
			h_cur
			, s_aclattr_main
			, h_ctx);

	ae2f_ifnezerr(h_ctx->m_err) {
		return CXChildVisit_Break;
	}

	unless(fnctx.m_state0 & ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING) {
		_aclutil_fnctx_sync(fnctx, fnctx, ACLUTIL_FNCTX_SNAPS_LBL);
	}

	ae2f_ifnezerr(h_ctx->m_err)	return CXChildVisit_Break;

	if(h_ctx->m_count.m_fnimpl) {
		_aclspv_grow_vec_with_copy(_aclspv_malloc, _aclspv_free, _aclspv_memcpy, L_new
				, h_ctx->m_section.m_fndef
				, (size_t)aclutil_count_to_sz(h_ctx->m_count.m_fndef + h_ctx->m_count.m_fnimpl)
				);

		ae2f_ifezerr(h_ctx->m_section.m_fndef.m_p) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			return CXChildVisit_Break;
		}

		memcpy(
				aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fndef) + h_ctx->m_count.m_fndef
				, h_ctx->m_section.m_fnimpl.m_p
				, (size_t)aclutil_count_to_sz(h_ctx->m_count.m_fnimpl)
		      );

		h_ctx->m_count.m_fndef += h_ctx->m_count.m_fnimpl;
	}

	unless(fnctx.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
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


	fnctx.m_ctx->m_work_data	= fnctx.m_ctx;
	fnctx.m_ctx->m_work_nxt		= aclemit_root;
	return CXChildVisit_Continue;
}
