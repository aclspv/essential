#include "./entp.h"
#include "./entp/args.h"
#include "./entp/cfg.h"
#include "./body.h"
#include "./../dtor.h"

#include <aclty.h>
#include <aclval.h>
#include <aclmem.h>
#include <aclctx/err.h>
#include <aclctx/id.h>

#include <aclcur/common.h>
#include <aclutil/capability.h>

#include <aclutil/emitx.h>
#include <aclsym/entp.h>
#include <aclutil/scale.h>
#include <util/log.h>

#include <ae2f/pat/dbg.h>

enum CXChildVisitResult aclemit_fndecl_entp(
		CXCursor h_cur,
		CXCursor h_parent,
		CXClientData h_ctx
		) {
	h_aclspv_ctx_t	ctx = h_ctx;
	aclsym_entp_t	entp = {0, };
	struct aclemit_fndecl_entp_arg_data DATA;
	const int NPARAMS = clang_Cursor_getNumArguments(h_parent);

	ae2fpat_assertassume(ctx);

	aclmemzero(&DATA, sizeof(DATA));
	DATA.m_ctx = ctx;
	DATA.m_idx_entp_buf_count = ctx->m_count.m_entp;

	if(NPARAMS < 0) {
		_aclctx_err(ctx, ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS);
		return CXChildVisit_Break;
	}

	entp = aclemit_fndecl_entp_cfg(h_parent, ctx);
	aclctx_err_check(ctx)
		return CXChildVisit_Break;

	clang_visitChildren(h_parent, aclemit_fndecl_entp_arg, &DATA);

	ae2f_ifezerr(aclutil_add_capability_by_flag(ctx, SpvCapabilityShader)) {
		_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
		return CXChildVisit_Break;
	}

	aclctx_err_check(ctx)
		return CXChildVisit_Break;

	{
		const aclspv_id_t	id_void_func = aclty_mk_fn(
				aclty_mk_default(
					ACLSPV_ID_VOID
					, ctx
					)
				, ctx
				);

		ae2f_ifezerr(id_void_func) {
			_aclctx_err(ctx, ACLSPV_COMPILE_ALLOC_FAILED_ID);
			return CXChildVisit_Break;
		}

		ae2f_ifezerr(ctx->m_count.m_fndef = aclutil_emitx_5(
					&ctx->m_section.m_fndef
					, ctx->m_count.m_fndef
					, SpvOpFunction
					, ACLSPV_ID_VOID
					, entp.m_id
					, 0
					, id_void_func
					)) {
			return CXChildVisit_Break;
		}
	}

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&ctx->m_fnlist.m_entp
				, (ctx->m_fnlist.m_num_entp + 1)
				, sizeof(entp)
				, ACLSPV_COMPILE_ALLOC_FAILED_ENTP
				)) return CXChildVisit_Break;

	((aclsym_entp_t* ae2f_restrict)ctx->m_fnlist.m_entp.m_p)
		[ctx->m_fnlist.m_num_entp++] = entp;

	{
		aclspv_bool_t			c_is_const = 0;
		aclspv_bool_t			c_is_pure = 0;
		enum CXChildVisitResult		ret;
		aclsym_entp_t* ae2f_restrict	p_entp;

		aclemit_dtor_begin(h_ctx);

		if(!aclemit_dtor_push(h_ctx))
			return CXChildVisit_Break;

		ret = aclemit_fndecl_bodycfg(
				h_cur
				, h_ctx
				, &c_is_const
				, &c_is_pure
				);

		/** an entry point has no function symbol: the purity snapshot,
		 * taken while the cursor arena was alive, lives on its own record. */
		p_entp = ctx->m_fnlist.m_num_entp
			? &((aclsym_entp_t* ae2f_restrict)ctx->m_fnlist.m_entp.m_p)
				[ctx->m_fnlist.m_num_entp - 1]
			: 0;

		if(p_entp) {
			if(c_is_const)
				p_entp->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST);
			if(c_is_pure)
				p_entp->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE);
		}

		return ret;
	}

	ae2f_unreachable();
}
