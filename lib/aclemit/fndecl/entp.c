#include "./entp.h"
#include "./entp/args.h"
#include "./entp/cfg.h"
#include "./body.h"

#include <aclmem.h>
#include <aclty.h>

#include <util/fnctx.h>
#include <util/constant.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <util/entp.h>
#include <util/scale.h>
#include <util/log.h>

#include <ae2f/pat/dbg.h>


enum CXChildVisitResult aclemit_fndecl_entp(ae2f_unused CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx) {
	h_aclspv_ctx_t	ctx = h_ctx;
	aclutil_entp_t	entp = {0, };
	struct aclemit_fndecl_entp_arg_data DATA;
	const int NPARAMS = clang_Cursor_getNumArguments(h_parent);

	ae2fpat_assertassume(ctx);

	aclmemzero(&DATA, sizeof(DATA));
	DATA.m_ctx = ctx;
	DATA.m_ctx->m_num_cursor = 0;
	DATA.m_idx_entp_buf_count = ctx->m_count.m_entp;

	if(NPARAMS < 0) {
		ctx->m_err = ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS;
		return CXChildVisit_Break;
	}

	entp = aclemit_fndecl_entp_cfg(h_parent, ctx);
	ae2f_ifnezerr(ctx->m_err)
		return CXChildVisit_Break;

	unless(NPARAMS) {
		goto LBL_HOLLOW_ENTP;
	}

	clang_visitChildren(h_parent, aclemit_fndecl_entp_arg, &DATA);
	ae2f_ifnezerr(ctx->m_err)
		return CXChildVisit_Break;

	if(DATA.m_push_size) {
		aclspv_id_t PSH_PTR_ID;

		ae2f_ifnezerr_strict((ctx->m_max_push_size += DATA.m_push_size) > ctx->m_usrcfg.m_max_push_size_limit) {
			ctx->m_err = ACLSPV_COMPILE_PUSHCONSTANT_TOO_BIG;
			return CXChildVisit_Break;
		}

		PSH_PTR_ID	= aclutil_mk_constant_ptr_psh_id(
				ctx->m_max_push_size
				, ctx
				);

		ae2f_ifezerr(PSH_PTR_ID) {
			unless(ctx->m_err) {
				err_puts("unhandled");
				ctx->m_err = ACLSPV_COMPILE_UNHANDLED;
			}

			return CXChildVisit_Break;
		}

		if(ctx->m_push_size_id) {
			((aclspv_wrd_t* ae2f_restrict)ctx->m_section.m_vars.m_p)
				[ctx->m_push_size_ty_pad] = PSH_PTR_ID;
		} else {
			dbg_puts("push size id is not set. having new one...");
			ctx->m_push_size_ty_pad = ctx->m_count.m_vars + 1;
			ae2f_ifezerr(ctx->m_count.m_vars = aclutil_emitx_4(
						&ctx->m_section.m_vars
						, ctx->m_count.m_vars
						, SpvOpVariable
						, PSH_PTR_ID
						, (ctx->m_push_size_id = ctx->m_id++)
						, SpvStorageClassPushConstant
						)) {
				return CXChildVisit_Break;
			}
		}

		entp.m_nprm = DATA.m_num_interface;
	}

LBL_HOLLOW_ENTP:

	{
		const aclspv_id_t	id_void_func = aclty_mk_fn(
				aclid_mk_default_id(
					ACLSPV_ID_VOID
					, ctx
					)
				, ctx
				);

		ae2f_ifezerr(id_void_func) {
			ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_ID;
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

	((aclutil_entp_t* ae2f_restrict)ctx->m_fnlist.m_entp.m_p)
		[ctx->m_fnlist.m_num_entp++] = entp;

	{
		aclutil_fnctx	fnctx;
		fnctx.m_retid = ACLSPV_ID_VOID;
		return aclemit_fndecl_bodycfg(h_cur, &fnctx, h_ctx);
	}
}
