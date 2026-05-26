#include "ae2f/c90/StdInt/enum.h"
#include <aclspv.h>

#include <clang-c/CXErrorCode.h>
#include <clang-c/Index.h>
#include <util/ctx.h>
#include <util/entp.h>
#include <util/fn.h>
#include <util/fnctx.h>
#include <util/iddef.h>
#include <util/cursor.h>
#include <util/scale.h>
#include <util/cxtp.h>
#include <util/emitx.h>

#include <emit/count_fn.h>
#include <emit/decl_glob_obj.h>
#include <emit/entp_body.h>

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_ctx_compile(h_aclspv_ctx_t h_ctx, CXCursor c_cur_root) {
	ae2f_ifezerr(h_ctx)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err) return h_ctx->m_err;

	clang_visitChildren(c_cur_root, aclemit_count_fn, &(*h_ctx));
	ae2f_ifnezerr(h_ctx->m_err) return h_ctx->m_err;

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, (*h_ctx).m_fnlist.m_entp, (size_t)(sizeof(util_entp_t) * (*h_ctx).m_fnlist.m_num_entp));
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, (*h_ctx).m_fnlist.m_fn, (size_t)(sizeof(lib_build_fn_t) * (*h_ctx).m_fnlist.m_num_fn));

	(*h_ctx).m_tmp.m_w3 = (*h_ctx).m_id; /** anchor */
	(*h_ctx).m_id += (*h_ctx).m_fnlist.m_num_entp + (*h_ctx).m_fnlist.m_num_fn;

	clang_visitChildren(c_cur_root, aclemit_iter_entry_point, &(*h_ctx));
	(*h_ctx).m_tmp.m_w0 = 0;
	clang_visitChildren(c_cur_root, aclemit_decl_glob_obj, &(*h_ctx));


	{
		aclspv_wrdcount_t	IDX = (*h_ctx).m_fnlist.m_num_entp;
		const aclspv_wrd_t	ANCHOR	= (*h_ctx).m_tmp.m_w3;
		const aclspv_id_t	VOID	= aclid_mk_default_id(ID_DEFAULT_VOID, &(*h_ctx));

		const aclspv_id_t	ID_DEFAULT_FN_VOID = aclutil_mk_cxtp_fn(
				ID_DEFAULT_VOID
				, &(*h_ctx)
				);

		ae2f_expected_but_else(VOID) {
			return h_ctx->m_err = (enum ACLSPV_COMPILE_)ae2f_bll_sel_bool_naive1(
					uint_fastenum_t
					, (uint_fastenum_t)h_ctx->m_err
					, (uint_fastenum_t)h_ctx->m_err
					, ACLSPV_COMPILE_ALLOC_FAILED_ID
					);
		}

		ae2f_expected_but_else(ID_DEFAULT_FN_VOID) {
			return h_ctx->m_err = (enum ACLSPV_COMPILE_)ae2f_bll_sel_bool_naive1(
					uint_fastenum_t
					, (uint_fastenum_t)h_ctx->m_err
					, (uint_fastenum_t)h_ctx->m_err
					, ACLSPV_COMPILE_ALLOC_FAILED_ID
					);
		}


		while((IDX--)) {
			((util_entp_t* ae2f_restrict)(*h_ctx).m_fnlist.m_entp.m_p)[IDX].m_id = ANCHOR + IDX;
		}

		IDX = (*h_ctx).m_fnlist.m_num_entp;
		while((IDX--)) {
			/**
			 * IDX must be w0 at the end
			 * w0: current entry point index
			 * v0: current variable table		(scale)
			 * */
#define	FNINFO	((util_entp_t* ae2f_restrict)(*h_ctx).m_fnlist.m_entp.m_p)[(*h_ctx).m_tmp.m_w0]
			x_scale* ae2f_restrict const	FNSCALE	= get_scale_from_vec(&(*h_ctx).m_scale_vars
					,  (size_t)((*h_ctx).m_tmp.m_w0 = IDX)
					);

			aclutil_fnctx	FNCTX;
			FNCTX.m_ctx	= &(*h_ctx);
			FNCTX.m_state0	= 0;
			FNCTX.m_retid	= ID_DEFAULT_VOID;
			FNCTX.m_var_stage	= 1;

			(*h_ctx).m_count.m_fnimpl = 0;

			ae2f_expected_but_else((*h_ctx).m_scale_vars.m_p && FNSCALE) {
				return (*h_ctx).m_err = ACLSPV_COMPILE_MET_INVAL;
			}

			(*h_ctx).m_tmp.m_w0 = IDX;

			ae2f_expected_but_else((*h_ctx).m_count.m_fndef = util_emitx_5(
						&(*h_ctx).m_section.m_fndef
						, (*h_ctx).m_count.m_fndef
						, SpvOpFunction
						, ID_DEFAULT_VOID
						, FNINFO.m_id
						, 0
						, ID_DEFAULT_FN_VOID
						)) {
				return (*h_ctx).m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			}

			ae2f_expected_but_else((*h_ctx).m_count.m_fndef = util_emitx_2(
						&(*h_ctx).m_section.m_fndef
						, (*h_ctx).m_count.m_fndef
						, SpvOpLabel
						, (*h_ctx).m_id++)) {
				return (*h_ctx).m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			}

			IDX = (aclspv_wrd_t)FNSCALE->m_sz / (size_t)sizeof(util_bind);
			(*h_ctx).m_num_cursor	= IDX;

			while(IDX--) {
				const util_bind* ae2f_restrict const BUFFER = get_buf_from_scale(&(*h_ctx).m_scale_vars, FNSCALE[0]);
				const aclspv_wrd_t CURSOR_IDX = aclutil_mk_cursor_base(
						(*h_ctx).m_num_cursor
						, &(*h_ctx).m_cursors
						, BUFFER[IDX].m_unified.m_cursor
						);

				assert((*h_ctx).m_scale_vars.m_p);
				assert(BUFFER);

				ae2f_unexpected_but_if(CURSOR_IDX == (*h_ctx).m_num_cursor + 1) {
					return h_ctx->m_err = (enum ACLSPV_COMPILE_)ae2f_bll_sel_bool_naive1(
							uint_fastenum_t
							, h_ctx->m_err
							, (uint_fastenum_t) h_ctx-> m_err
							, ACLSPV_COMPILE_UNHANDLED
							);
				}

				if(CURSOR_IDX == (*h_ctx).m_num_cursor)
					++(*h_ctx).m_num_cursor;

				CURSOR_IDX[(util_cursor* ae2f_restrict)(*h_ctx).m_cursors.m_p]
					.m_data.m_prm_decl.m_info = BUFFER[IDX];
			}
			IDX = (*h_ctx).m_tmp.m_w0;


#undef	FNINFO

			FNCTX.m_state0 = 0;
			clang_visitChildren(((util_entp_t* ae2f_restrict)(*h_ctx).m_fnlist.m_entp.m_p)[IDX].m_fn
					, aclemit_entp_body
					, &FNCTX);

			unless(FNCTX.m_state0 & ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING) {
				(*h_ctx).m_count.m_fnimpl = FNCTX.m_lbl_snapshot;
			}

			ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

			if((*h_ctx).m_count.m_fnimpl) {
				_aclspv_grow_vec_with_copy(_aclspv_malloc, _aclspv_free, _aclspv_memcpy, L_new
						, (*h_ctx).m_section.m_fndef
						, (size_t)count_to_sz((*h_ctx).m_count.m_fndef + (*h_ctx).m_count.m_fnimpl)
						);

				ae2f_expected_but_else((*h_ctx).m_section.m_fndef.m_p) {
					return (*h_ctx).m_err = ACLSPV_COMPILE_ALLOC_FAILED;
				}

				memcpy(
						get_wrd_of_vec(&(*h_ctx).m_section.m_fndef) + (*h_ctx).m_count.m_fndef
						, (*h_ctx).m_section.m_fnimpl.m_p
						, (size_t)count_to_sz((*h_ctx).m_count.m_fnimpl)
				      );

				(*h_ctx).m_count.m_fndef += (*h_ctx).m_count.m_fnimpl;
			}


			unless(FNCTX.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
				ae2f_expected_but_else((*h_ctx).m_count.m_fndef = emit_opcode(
							&(*h_ctx).m_section.m_fndef
							, (*h_ctx).m_count.m_fndef
							, SpvOpReturn, 0
							)) {
					return (*h_ctx).m_err = ACLSPV_COMPILE_ALLOC_FAILED;
				}
			}

			ae2f_expected_but_else((*h_ctx).m_count.m_fndef = emit_opcode(
						&(*h_ctx).m_section.m_fndef
						, (*h_ctx).m_count.m_fndef
						, SpvOpFunctionEnd, 0
						)) {
				return (*h_ctx).m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			}

			assert(IDX == (*h_ctx).m_tmp.m_w0);
			{ ae2f_assume(IDX == (*h_ctx).m_tmp.m_w0); }
		}
	}


	return h_ctx->m_err;
}

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile1(
		const h_aclspv_ctx_t	h_ctx,
		CXTranslationUnit	h_tu
		) {
	unsigned	CXTU_IDX_ERR;

	ae2f_ifezerr(h_ctx)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(h_tu)	return ACLSPV_COMPILE_ERR_CXTU;

	CXTU_IDX_ERR = clang_getNumDiagnostics(h_tu);
	if(h_ctx->m_flag0 & ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS) while(CXTU_IDX_ERR-- && !h_ctx->m_err) {
		const CXDiagnostic DIAG = clang_getDiagnostic(h_tu, CXTU_IDX_ERR);
		enum CXDiagnosticSeverity SEVERITY = clang_getDiagnosticSeverity(DIAG);

		CXString TXT = clang_formatDiagnostic(DIAG, clang_defaultDiagnosticDisplayOptions());

		dbg_puts(TXT.data);

		switch(SEVERITY) {
			case CXDiagnostic_Note:
				dbg_puts("NOTE");
				break;
			case CXDiagnostic_Error:
				if(h_ctx->m_flag0 & ACLSPV_CTX_FLAG0_KILL_ON_DIAGNOSTICS) 
					h_ctx->m_err = ACLSPV_COMPILE_DIAG_ERR;
				dbg_puts("_ERR");
				break;

			case CXDiagnostic_Warning:
				dbg_puts("WARN");
				break;
			case CXDiagnostic_Ignored:
				dbg_puts("IGNR");
				break;

			case CXDiagnostic_Fatal:
				if(h_ctx->m_flag0 & ACLSPV_CTX_FLAG0_KILL_ON_DIAGNOSTICS)
					h_ctx->m_err = ACLSPV_COMPILE_DIAG_FTAL;
				dbg_puts("FTAL");
				break;
			default:
				ae2f_unreachable();
		}

		clang_disposeString(TXT);
	}

	return aclspv_ctx_compile(h_ctx, clang_getTranslationUnitCursor(h_tu));
}

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile2(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt,
		CXIndex							h_cxidx
		) {
	CXTranslationUnit	CXTU;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(h_cxidx)		return ACLSPV_COMPILE_ERR_CXIDX;
	ae2f_ifezerr(rdwr_unsaved)	return ACLSPV_COMPILE_PRM_INVAL_NIL;

	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	{
		enum CXErrorCode CXERR = clang_parseTranslationUnit2(
				h_cxidx, ae2f_NIL
				, (const char**)rd_argv_opt
				, c_argc
				, rdwr_unsaved, c_unsaved_count
				, CXTranslationUnit_None, &CXTU
				);

		if(rwr_cxerr_opt)	*rwr_cxerr_opt = CXERR;
	}

	ae2f_ifezerr(CXTU)		return h_ctx->m_err = ACLSPV_COMPILE_ERR_CXTU;

	h_ctx->m_err = aclspv_ctx_compile1(h_ctx, CXTU);
	clang_disposeTranslationUnit(CXTU);

	return h_ctx->m_err;
}

ae2f_extern ACLSPV_ABI_DECL enum ACLSPV_COMPILE_ aclspv_ctx_compile3(
		const h_aclspv_ctx_t	h_ctx,
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
		) {
	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(rdwr_unsaved)	return ACLSPV_COMPILE_PRM_INVAL_NIL;

	{
		const CXIndex CXIDX		= clang_createIndex(0, 0);
		ae2f_ifezerr(CXIDX)		{ 
			clang_disposeIndex(CXIDX);
			return h_ctx->m_err = ACLSPV_COMPILE_ERR_CXIDX; 
		}
		h_ctx->m_err = aclspv_ctx_compile2(h_ctx, rdwr_unsaved, c_unsaved_count, rd_argv_opt, c_argc, rwr_cxerr_opt, CXIDX);
		clang_disposeIndex(CXIDX);
	}

	return h_ctx->m_err;
}
