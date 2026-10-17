#include "./cfg.h"

#include <aclctx/err.h>

#include <aclattr/wrkgroup.h>
#include <aclattr/execmodel.h>

#include <aclmem.h>

#include <util/constant.h>
#include <util/emitx.h>
#include <util/iddef.h>


typedef struct {
	h_aclspv_ctx_t	m_ctx;
	aclspv_id_t	m_entp;
} s_execmode_t;

static enum CXChildVisitResult s_execmode(CXCursor c, ae2f_unused CXCursor u0, CXClientData h) {
	s_execmode_t* ae2f_restrict p = h;
	CXString attr;
	const char* ndl;

	aclspv_wrd_t	execmode;
	aclspv_wrd_t	num_operands;
	aclspv_wrd_t	buf[3 + 3] = {0, };
	aclspv_bool_t	is_spec[3] = {0, };
	aclspv_bool_t	needs_id = 0;

	if(c.kind != CXCursor_AnnotateAttr)
		return CXChildVisit_Continue;

	attr = clang_getCursorSpelling(c);

	if((ndl = strstr(attr.data, "aclspv_execmode0"))) {
		unsigned long opcode;
		sscanf(ndl
				, "aclspv_execmode0 ( %lu ) "
				, &opcode
				);

		num_operands = 0;
		execmode = (aclspv_wrd_t)opcode;
	}

	else if((ndl = strstr(attr.data, "aclspv_execmode1"))) {
		unsigned long opcode;
		unsigned long o0;
		char b0;

		sscanf(ndl
				, "aclspv_execmode1 ( %lu , %c%lu ) "
				, &opcode
				, &b0
				, &o0
				);

		num_operands = 1;
		execmode = (aclspv_wrd_t)opcode;

		buf[3 + 0] = (aclspv_wrd_t)o0;
		is_spec[0] = b0 == '&';
	}

	else if((ndl = strstr(attr.data, "aclspv_execmode2"))) {
		unsigned long opcode;
		unsigned long o0;
		unsigned long o1;
		char b0;
		char b1;

		sscanf(ndl
				, "aclspv_execmode2 ( %lu , %c%lu , %c%lu ) "
				, &opcode
				, &b0
				, &o0
				, &b1
				, &o1
				);

		num_operands = 2;
		execmode = (aclspv_wrd_t)opcode;

		buf[3 + 0] = (aclspv_wrd_t)o0;
		buf[3 + 1] = (aclspv_wrd_t)o1;

		is_spec[0] = b0 == '&';
		is_spec[1] = b1 == '&';
	}

	else if((ndl = strstr(attr.data, "aclspv_execmode3"))) {
		unsigned long opcode;
		unsigned long o0;
		unsigned long o1;
		unsigned long o2;
		char b0;
		char b1;
		char b2;

		sscanf(ndl
				, "aclspv_execmode2 ( %lu , %c%lu , %c%lu , %c%lu ) "
				, &opcode
				, &b0, &o0
				, &b1, &o1
				, &b2, &o2
				);

		num_operands = 3;
		execmode = (aclspv_wrd_t)opcode;

		buf[3 + 0] = (aclspv_wrd_t)o0;
		buf[3 + 1] = (aclspv_wrd_t)o1;
		buf[3 + 2] = (aclspv_wrd_t)o2;

		is_spec[0] = b0 == '&';
		is_spec[1] = b1 == '&';
		is_spec[2] = b2 == '&';
	} else goto END;

	dbg_puts("s_execmode\n");

	{
		aclspv_wrd_t i;
		for(i = num_operands; i --> 0; )
			needs_id |= is_spec[i];
	}

	buf[0] = needs_id ? SpvOpExecutionModeId : SpvOpExecutionMode;
	buf[0] = aclspv_opcode_to_wrd(buf[0], 2 + num_operands);
	buf[1] = p->m_entp;
	buf[2] = execmode;

	ae2f_ifnezerr(needs_id && p->m_ctx->m_usrcfg.m_ver_spv < aclspv_ver_spv(1, 2)) {
		_aclctx_err(p->m_ctx, ACLSPV_COMPILE_NOIMPL_V_WRKGROUP_SIZE);
		err_puts("execution mode id is for spirv 1.2");
		goto ERR;
	}

	if(needs_id) {
		aclspv_wrd_t i;

		for(i = num_operands; i --> 0; ) {
			if(is_spec[i]) {
				aclval_spec_t nd = aclval_get_spec(p->m_ctx, buf[3 + i]);
				ae2f_ifezerr(nd.m_val) goto ERR;
				if(nd.m_val) {								
					ae2f_ifezerr_strict(aclid_is_int((B_id_default_t) nd.m_ty)) {
						p->m_ctx->m_err = ACLSPV_COMPILE_WORKGROUP_SIZE_NO_INT;
						goto ERR;
					}						
					buf[3 + i]	= nd.m_val;		
				} else {							
					ae2f_ifezerr(buf[3 + i] = aclutil_mk_constant_spec_id(
								buf[3 + i], 1, p->m_ctx)) {	
						goto ERR;
					}								
				}
			} ae2f_ifezerr(buf[3 + i] = aclutil_mk_constant_val_id(		
						buf[3 + i]
						, p->m_ctx)) {
				goto ERR;
			}
		}
	}

	{
		aclspv_wrd_t off_execmode;
		aclspv_wrd_t* ae2f_restrict p_ex;

		dbg_puts("execmode buf \n");

		ae2f_ifezerr(p->m_ctx->m_count.m_execmode = aclutil_emit_wrd(
				&p->m_ctx->m_section.m_execmode
				, (off_execmode = p->m_ctx->m_count.m_execmode) + 2 + num_operands
				, 0
				)) goto ERR;

		p_ex = p->m_ctx->m_section.m_execmode.m_p;
		aclmemcpy(&p_ex[off_execmode], &buf[0]
				, (3 + num_operands) * (aclspv_wrd_t)sizeof(aclspv_wrd_t));
	}

END:
	clang_disposeString(attr);
	return CXChildVisit_Continue;

ERR:
	_aclctx_err(p->m_ctx, ACLSPV_COMPILE_UNHANDLED);
	clang_disposeString(attr);
	return CXChildVisit_Break;
}

aclutil_entp_t aclemit_fndecl_entp_cfg(CXCursor h_parent, h_aclspv_ctx_t CTX) {
	const CXString NAME = clang_getCursorSpelling(h_parent);
	struct	aclattr_wrkgroup_prm	XYZ;
	enum SpvExecutionModel_	EXEC_MODEL = SpvExecutionModelMax;
	aclutil_entp_t		THIS_ENTRY_POINT;
	aclspv_wrdcount_t	POS = CTX->m_count.m_entp;

	aclmemzero(&THIS_ENTRY_POINT, sizeof(THIS_ENTRY_POINT));

	ae2f_ifnezerr(CTX->m_err) return THIS_ENTRY_POINT;

	clang_visitChildren(h_parent, s_attr_execmodel, &EXEC_MODEL);

	if(EXEC_MODEL == SpvExecutionModelMax)
		EXEC_MODEL = SpvExecutionModelGLCompute;	/** fallback */

	if(EXEC_MODEL == SpvExecutionModelGLCompute || EXEC_MODEL == SpvExecutionModelKernel) {
		XYZ.m_0  = 1;
		XYZ.m_1  = 1;
		XYZ.m_2  = 1;
		XYZ.m_flag_specid  = 0;
		clang_visitChildren(h_parent, aclattr_wrkgroup, &XYZ);

#define	S_GETSPECID(K_idx)										\
		if(XYZ.m_flag_specid & (1 << K_idx)) {						\
			aclval_spec_t nd = aclval_get_spec(CTX, XYZ.m_##K_idx);	\
			ae2f_ifezerr(nd.m_val) goto LBL_ABRT;						\
			if(nd.m_val) {								\
				ae2f_ifezerr_strict(aclid_is_int((B_id_default_t) nd.m_ty)) {	\
					CTX->m_err = ACLSPV_COMPILE_WORKGROUP_SIZE_NO_INT;	\
					goto LBL_ABRT;						\
				}								\
				XYZ.m_##K_idx	= nd.m_val;					\
			} else {								\
				ae2f_ifezerr(XYZ.m_##K_idx = aclutil_mk_constant_spec_id(	\
							XYZ.m_##K_idx, 1, CTX)) {		\
					goto LBL_ABRT;						\
				}								\
			}									\
		} else ae2f_ifezerr(XYZ.m_##K_idx = aclutil_mk_constant_val_id(					\
					(aclspv_wrd2_t)XYZ.m_##K_idx					\
					, CTX)) {						\
			goto LBL_ABRT;								\
		}

		if(XYZ.m_flag_specid) {
			ae2f_ifnezerr(CTX->m_usrcfg.m_ver_spv < aclspv_ver_spv(1, 2)) {
				CTX->m_err = ACLSPV_COMPILE_NOIMPL_V_WRKGROUP_SIZE;
				err_puts("workgroup size id is for spirv 1.2");
				goto LBL_ABRT;
			}

			S_GETSPECID(0);
			S_GETSPECID(1);
			S_GETSPECID(2);
		}
#undef	S_GETSPECID
	}

	THIS_ENTRY_POINT.m_fn	= h_parent;
	THIS_ENTRY_POINT.m_id	= aclctx_newid(CTX);

	{
		s_execmode_t ex;
		ex.m_ctx = CTX;
		ex.m_entp = THIS_ENTRY_POINT.m_id;
		clang_visitChildren(h_parent, s_execmode, &ex);
		ae2f_ifnezerr(CTX->m_err)
			goto LBL_ABRT;
	}

#if	1
	/** OpEntryPoint */

	unless((CTX->m_count.m_entp = emit_opcode(
					&CTX->m_section.m_entp
					, CTX->m_count.m_entp
					, SpvOpEntryPoint, 0)))
		goto LBL_ABRT;
	unless((CTX->m_count.m_entp = aclutil_emit_wrd(
					&CTX->m_section.m_entp
					, CTX->m_count.m_entp
					, (uint_least32_t) (EXEC_MODEL))))
		goto LBL_ABRT;
	unless((CTX->m_count.m_entp = aclutil_emit_wrd(
					&CTX->m_section.m_entp
					, CTX->m_count.m_entp
					, THIS_ENTRY_POINT.m_id)))
		goto LBL_ABRT;
	unless((CTX->m_count.m_entp = aclutil_emit_str(
					&CTX->m_section.m_entp
					, CTX->m_count.m_entp
					, NAME.data)))
		goto LBL_ABRT;

	set_oprnd_count_for_opcode(
			aclutil_get_wrd_of_vec(&CTX->m_section.m_entp)[POS]
			, CTX->m_count.m_entp - POS - 1
			);
#endif

	/** OpExecMode */
	if(EXEC_MODEL == SpvExecutionModelGLCompute || EXEC_MODEL == SpvExecutionModelKernel)
		ae2f_expected_but_else(CTX->m_count.m_execmode = aclutil_emitx_6(
					&CTX->m_section.m_execmode
					, CTX->m_count.m_execmode
					, XYZ.m_flag_specid ? SpvOpExecutionModeId : SpvOpExecutionMode
					, THIS_ENTRY_POINT.m_id
					, SpvExecutionModeLocalSize
					, XYZ.m_0, XYZ.m_1, XYZ.m_2
					)) goto LBL_ABRT;

	CTX->m_err = ACLSPV_COMPILE_OK;

LBL_ABRT:
	clang_disposeString(NAME);
	return THIS_ENTRY_POINT;
}
