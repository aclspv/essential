/** @file alcpsv.c */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <aclspv.h>
#include <.aclspv/util/vcopr.h>

#include <clang-c/CXErrorCode.h>
#include <clang-c/Index.h>

#include "./impl/conf.h"

#include "./util/ctx.h"
#include "./util/entp.h"
#include "./util/fn.h"
#include "./util/fnctx.h"
#include "./util/iddef.h"
#include "./util/cursor.h"
#include "./util/scale.h"
#include "./util/cxtp.h"

#include "./emit/count_fn.h"
#include "./emit/decl_glob_obj.h"
#include "./emit/entp_body.h"


ACLSPV_ABI_IMPL void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		) {
	enum ACLSPV_COMPILE_	E = d_ctx->m_err;

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_constant_cache);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_type_uniques);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_cursors);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_scale_vars);

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_fnlist.m_entp);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_fnlist.m_fn);

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v0);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v1);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v2);

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_capability);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_decorate);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_entp);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_fndef);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_execmode);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_ext);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_memmodel);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_name);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_types);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_vars);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_fnimpl);

	memset(d_ctx, 0, sizeof(x_aclspv_ctx));
	d_ctx->m_err = E;
}

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_wrd_t	c_init_size_exp,
		const aclspv_wrd_t	c_init_flag0,
		const aclspv_wrd_t	c_spirv_ver
		) {
	ae2f_ifezerr(i_ctx)
		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	memset(i_ctx, 0, sizeof(x_aclspv_ctx));
	i_ctx->m_flag0		|= c_init_flag0;
	i_ctx->m_spirv_ver	|= c_spirv_ver;

#define	ISZEXP2	ae2f_bll_sel_bool_naive1(size_t, c_init_size_exp, c_init_size_exp, 10)
#define	INITSZ	((size_t)(1 << ISZEXP2))

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_type_uniques		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_scale_vars		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_cursors			, INITSZ);

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_capability	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_entp		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_ext		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_vars		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_decorate	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_execmode	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_memmodel	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_fndef		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_types		, INITSZ);

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v0		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v1		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v2		, INITSZ);

#undef	ISZEXP2

	ae2f_ifezerr(init_scale(&i_ctx->m_scale_vars, 0)) {
		aclspv_fini_ctx(i_ctx);
		return i_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
	}

	return ae2f_unexpected(i_ctx->m_err)
		? i_ctx->m_err
		: ae2f_unexpected(i_ctx->m_err = impl_conf(i_ctx));
}

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_ctx_compile(h_aclspv_ctx_t h_ctx, CXCursor CXROOTCUR) {
	ae2f_ifezerr(h_ctx)
		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	ae2f_ifnezerr(h_ctx->m_err) return h_ctx->m_err;

	clang_visitChildren(CXROOTCUR, aclemit_count_fn, &(*h_ctx));
	ae2f_ifnezerr(h_ctx->m_err) return h_ctx->m_err;

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, (*h_ctx).m_fnlist.m_entp, (size_t)(sizeof(util_entp_t) * (*h_ctx).m_fnlist.m_num_entp));
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, (*h_ctx).m_fnlist.m_fn, (size_t)(sizeof(lib_build_fn_t) * (*h_ctx).m_fnlist.m_num_fn));

	(*h_ctx).m_tmp.m_w3 = (*h_ctx).m_id; /** anchor */
	(*h_ctx).m_id += (*h_ctx).m_fnlist.m_num_entp + (*h_ctx).m_fnlist.m_num_fn;

	clang_visitChildren(CXROOTCUR, aclemit_iter_entry_point, &(*h_ctx));
	(*h_ctx).m_tmp.m_w0 = 0;
	clang_visitChildren(CXROOTCUR, aclemit_decl_glob_obj, &(*h_ctx));


	{
		aclspv_wrdcount_t	IDX = (*h_ctx).m_fnlist.m_num_entp;
		const aclspv_wrd_t	ANCHOR	= (*h_ctx).m_tmp.m_w3;
		const aclspv_id_t	VOID	= aclid_mk_default_id(ID_DEFAULT_VOID, &(*h_ctx));

		const aclspv_id_t	ID_DEFAULT_FN_VOID = aclutil_mk_cxtp_fn_no_prm(
				ID_DEFAULT_VOID
				, &(*h_ctx)
				);

		ae2f_expected_but_else(VOID) {
			return h_ctx->m_err = ae2f_bll_sel_bool_naive1(
					uint_fastenum_t
					, h_ctx->m_err
					, h_ctx->m_err
					, ACLSPV_COMPILE_ALLOC_FAILED_ID
					);
		}

		ae2f_expected_but_else(ID_DEFAULT_FN_VOID) {
			return h_ctx->m_err = ae2f_bll_sel_bool_naive1(
					uint_fastenum_t
					, h_ctx->m_err
					, h_ctx->m_err
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
					return h_ctx->m_err = ae2f_bll_sel_bool_naive1(
							uint_fastenum_t
							, h_ctx->m_err
							, h_ctx->m_err
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


/**
 * @fn	aclspv_compile
 * @brief	compile files and generate spir-v for vulkan
 * @returns	<INIT:aclspv_free_obj>
 * */
	ACLSPV_ABI_IMPL e_aclspv_compile_t 
aclspv_compile(
		struct CXUnsavedFile* ae2f_restrict const 	rdwr_unsaved,
		const unsigned					c_unsaved_count,
		const char* ae2f_restrict const * ae2f_restrict const	rd_argv_opt,
		const int						c_argc,
		aclspv_wrdcount_t* ae2f_restrict			rwr_output_count_opt,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict		rwr_output,
		enum CXErrorCode* ae2f_restrict				rwr_cxerr_opt
	      ) 
{
	const CXIndex CXIDX		= clang_createIndex(0, 0);
	CXTranslationUnit	CXTU	= ae2f_NIL;
	CXCursor		CXROOTCUR;

	enum CXErrorCode	CXERR	= CXError_Success;
	x_aclspv_ctx	CTX; 

	unsigned	CXTU_IDX_ERR;
#define	STATE_VAL	CTX.m_err

	ae2f_ifezerr(rdwr_unsaved) {
		err_puts("`rdwr_unsaved` is null");
		STATE_VAL = ACLSPV_COMPILE_PRM_INVAL_NIL;
		goto LBL_ERRCLEANUP;
	}

	ae2f_ifezerr(rwr_output) {
		err_puts("`rwr_output` is null");
		STATE_VAL = ACLSPV_COMPILE_PRM_INVAL_NIL;
		goto LBL_ERRCLEANUP;
	}

	ae2f_ifnezerr(aclspv_init_ctx(
				&CTX
				, 30
				, ACLSPV_CTX_FLAG0_SYMBOL | ACLSPV_CTX_FLAG0_OGL | ACLSPV_CTX_FLAG0_LOGICAL
				, 0x10000
				)) return CTX.m_err;

	unless(ae2f_expected(CXIDX)) {
		CTX.m_err = ACLSPV_COMPILE_ERR_CLANGIDX;
		goto LBL_CLEANUP;
	}

	CXERR = clang_parseTranslationUnit2(
			CXIDX, ae2f_NIL
			, (const char**)rd_argv_opt
			, c_argc
			, rdwr_unsaved, c_unsaved_count
			, CXTranslationUnit_None, &CXTU
			);

	CXTU_IDX_ERR = clang_getNumDiagnostics(CXTU);
	while(CXTU_IDX_ERR--) {
		const CXDiagnostic DIAG = clang_getDiagnostic(CXTU, CXTU_IDX_ERR);
		enum CXDiagnosticSeverity SEVERITY = clang_getDiagnosticSeverity(DIAG);

		CXString TXT = clang_formatDiagnostic(DIAG, clang_defaultDiagnosticDisplayOptions());

		dbg_puts(TXT.data);

		switch(SEVERITY) {
			case CXDiagnostic_Note:
				dbg_puts("NOTE");
				break;
			case CXDiagnostic_Error:
				dbg_puts("_ERR");
				break;
			case CXDiagnostic_Warning:
				dbg_puts("WARN");
				break;
			case CXDiagnostic_Ignored:
				dbg_puts("IGNR");
				break;
			case CXDiagnostic_Fatal:
				dbg_puts("FTAL");
				break;
			default:
				assert(0);
				ae2f_unreachable();
		}

		clang_disposeString(TXT);
	}

	if(ae2f_expected_not(CXERR)) {
		CTX.m_err = ACLSPV_COMPILE_ERR_CLANG;
		goto LBL_CLEANUP;
	}

	unless(ae2f_expected(CXTU)) {
		CTX.m_err = ACLSPV_COMPILE_MET_INVAL;
		goto LBL_CLEANUP;
	}

	CXROOTCUR = clang_getTranslationUnitCursor(CXTU);
	ae2f_ifnezerr(aclspv_ctx_compile(&CTX, CXROOTCUR))
		goto LBL_ERRCLEANUP;

	if(ae2f_expected_not(STATE_VAL = aclspv_dump_ctx_output(
					&CTX
					, rwr_output_count_opt
					, rwr_output)))
		goto LBL_ERRCLEANUP;

	goto LBL_CLEANUP;

LBL_ERRCLEANUP:
	CTX.m_err = ae2f_bll_sel_bool_naive1(
			uint_fastenum_t
			, CTX.m_err
			, CTX.m_err
			, ACLSPV_COMPILE_UNHANDLED
			);

LBL_CLEANUP:
	clang_disposeTranslationUnit(CXTU);
	aclspv_fini_ctx(&CTX);

	if(ae2f_expected(CXIDX)) clang_disposeIndex(CXIDX);
	if(ae2f_expected(rwr_cxerr_opt)) *rwr_cxerr_opt = CXERR;

	return CTX.m_err;
}

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_dump_ctx_output(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const			r_count,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict const	ret
		) {	
	aclspv_wrd_t			NUM_RET;
	aclspv_wrd_t* ae2f_restrict	RET;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	NUM_RET	
		= h_ctx->m_count.m_capability
		+ h_ctx->m_count.m_ext
		+ h_ctx->m_count.m_memmodel
		+ h_ctx->m_count.m_entp
		+ h_ctx->m_count.m_execmode
		+ h_ctx->m_count.m_name
		+ h_ctx->m_count.m_decorate
		+ h_ctx->m_count.m_types
		+ h_ctx->m_count.m_vars
		+ h_ctx->m_count.m_fndef
		+ 5;

	if(r_count)		*r_count = NUM_RET;
	unless(ret)		return ACLSPV_COMPILE_OK;

	*ret = RET = calloc(NUM_RET, sizeof(aclspv_wrd_t));
	ae2f_ifezerr(RET)	return ACLSPV_COMPILE_ALLOC_FAILED_ASM;

	RET[0]	|=	SpvMagicNumber;
	RET[1]	|=	h_ctx->m_spirv_ver;
	/** RET[2] is 0 */
	RET[3]	|=	NUM_RET;
	/** RET[4] is 0 */

#define copybinary(k_key, c_buf)	if(h_ctx->m_count.k_key)	\
	_aclutil_vcopr(k_key, |=, c_buf, h_ctx->m_section.k_key.m_p, h_ctx->m_count.k_key)

	copybinary(m_capability,	RET + 5);

	copybinary(m_ext,		RET + 5 
			+ h_ctx->m_count.m_capability
		  );

	copybinary(m_memmodel,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
		  );

	copybinary(m_entp,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
		  );

	copybinary(m_execmode,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
		  );

	copybinary(m_name,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
		  );

	copybinary(m_decorate,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
		  );

	copybinary(m_types,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
			+ h_ctx->m_count.m_decorate
		  );

	copybinary(m_vars,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
			+ h_ctx->m_count.m_decorate
			+ h_ctx->m_count.m_types
		  );

	copybinary(m_fndef,		RET + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
			+ h_ctx->m_count.m_decorate
			+ h_ctx->m_count.m_types
			+ h_ctx->m_count.m_vars
		  );

#undef	copybinary

	return ACLSPV_COMPILE_OK;
}
