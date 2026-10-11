#include "./entp.h"
#include "./entp/args.h"
#include "./entp/cfg.h"
#include "./body.h"

#include <aclmem.h>
#include <aclty.h>
#include <aclctx/err.h>
#include <aclctx/id.h>

#include <aclcur/common.h>
#include <aclutil/capability.h>

#include <util/fnctx.h>
#include <util/constant.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <util/entp.h>
#include <util/scale.h>
#include <util/log.h>

#include <ae2f/pat/dbg.h>

enum CXChildVisitResult aclemit_fndecl_entp(
		CXCursor h_cur,
		CXCursor h_parent,
		CXClientData h_ctx
		) {
	h_aclspv_ctx_t	ctx = h_ctx;
	aclutil_entp_t	entp = {0, };
	struct aclemit_fndecl_entp_arg_data DATA;
	const int NPARAMS = clang_Cursor_getNumArguments(h_parent);

	ae2fpat_assertassume(ctx);

	aclmemzero(&DATA, sizeof(DATA));
	DATA.m_ctx = ctx;
	DATA.m_idx_entp_buf_count = ctx->m_count.m_entp;

	if(NPARAMS < 0) {
		ctx->m_err = ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS;
		return CXChildVisit_Break;
	}

	entp = aclemit_fndecl_entp_cfg(h_parent, ctx);
	ae2f_ifnezerr(ctx->m_err)
		return CXChildVisit_Break;

	clang_visitChildren(h_parent, aclemit_fndecl_entp_arg, &DATA);

	ae2f_ifezerr(aclutil_add_capability_by_flag(ctx, SpvCapabilityShader)) {
		_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
		return CXChildVisit_Break;
	}

	ae2f_ifnezerr(ctx->m_err)
		return CXChildVisit_Break;

	if(DATA.m_id_push_constant_struct_ty) {
		aclspv_wrd_t* ae2f_restrict p_ty = &aclutil_get_wrd_of_vec(&ctx->m_section.m_vars)[ctx->m_push_size_ty_pad];
		aclspv_id_t id_cfg;
		aclty_t* ae2f_restrict p_cfg;

		p_cfg = aclty_find(*p_ty, h_ctx);

		dbg_prefix(a);
		dbg_call(fprintf, (
					stderr
					, "push constant member type: %u\n"
					, (unsigned)(p_cfg ? p_cfg->m_info.m_ptr.m_type : 0)
				  ));

		ae2f_ifezerr(id_cfg = aclty_cfg_struct2(h_ctx, p_cfg ? p_cfg->m_info.m_ptr.m_type : 0)) {
			_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED);
			return CXChildVisit_Break;
		}

		ae2f_ifezerr(aclty_cfg_struct_dcr1(id_cfg, SpvDecorationBlock, h_ctx)) {
			_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED);
			return CXChildVisit_Break;
		}

		ae2f_ifezerr(p_cfg = aclty_find(id_cfg, h_ctx)) {
			_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED);
			return CXChildVisit_Break;
		}

		dbg_prefix(a);
		dbg_call(fprintf, (
					stderr
					, "push constant member index: %u\n"
					, (unsigned)p_cfg->m_info.m_struct.m_num_mbr
				  ));

		ae2f_ifezerr( aclty_add_struct_mbr_std430(p_cfg, DATA.m_id_push_constant_struct_ty, h_ctx) ) {
			_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED);
			return CXChildVisit_Break;
		}

		ae2f_ifezerr(p_cfg = aclty_build_struct(id_cfg, h_ctx)) {
			_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED);
			return CXChildVisit_Break;
		}

		dbg_prefix(a);
		dbg_call(fprintf, (
					stderr
					, "push constant member index (after): %u\n"
					, (unsigned)p_cfg->m_info.m_struct.m_num_mbr
				  ));

		p_ty = &aclutil_get_wrd_of_vec(&ctx->m_section.m_vars)[ctx->m_push_size_ty_pad];

		ae2f_ifezerr(*p_ty = aclty_mk_ptr(p_cfg->m_type_id, SpvStorageClassPushConstant, ctx)) {
			_aclctx_err(ctx, ACLSPV_COMPILE_UNHANDLED);
			return CXChildVisit_Break;
		}

		++ctx->m_num_push_constant_func;

		/** the packing-level view's type was captured mid-build (aclty
		 * struct ids churn per arg): patch it to the final packed struct
		 * type now that the last arg settled it. */
		if(DATA.m_id_push_constant_reg_view) {
			h_aclcur_t p_r1 = aclcur_get_node(
					DATA.m_id_push_constant_reg_view
					, ctx
					);
			if(p_r1 && p_r1->m_class == ACLCUR_REG)
				p_r1->m_ty = DATA.m_id_push_constant_struct_ty;
		}
	}

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

	ae2f_unreachable();
}
