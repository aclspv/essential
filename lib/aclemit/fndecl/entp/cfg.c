#include "./cfg.h"
#include <ae2f/cc/memset.h>

#include <aclattr/wrkgroup.h>
#include <aclattr/execmodel.h>

#include <util/constant.h>
#include <util/emitx.h>
#include <util/iddef.h>

aclutil_entp_t aclemit_fndecl_entp_cfg(CXCursor h_parent, h_aclspv_ctx_t CTX) {
	const CXString NAME = clang_getCursorSpelling(h_parent);
	struct	aclattr_wrkgroup_prm	XYZ;
	enum SpvExecutionModel_	EXEC_MODEL = SpvExecutionModelMax;
	aclutil_entp_t		THIS_ENTRY_POINT;

	ae2fcc_memset(&THIS_ENTRY_POINT, 0, sizeof(THIS_ENTRY_POINT));

	aclspv_wrdcount_t	POS = CTX->m_count.m_entp;

	ae2f_ifnezerr(CTX->m_err) return THIS_ENTRY_POINT;
	CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED;

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
			aclutil_specconstant_val_t nd = aclutil_get_specconstant_val_id(CTX, XYZ.m_##K_idx);	\
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
					CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED;		\
					goto LBL_ABRT;						\
				}								\
			}									\
		} else ae2f_ifezerr(aclutil_mk_constant_val_id(					\
					(uint_least64_t)XYZ.m_0					\
					, CTX)) {						\
			CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED;				\
			goto LBL_ABRT;								\
		}

		if(XYZ.m_flag_specid) {
			ae2f_ifnezerr(CTX->m_spirv_ver < aclspv_ver_spv(1, 2)) {
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
	THIS_ENTRY_POINT.m_id	= CTX->m_id++;

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


	if(CTX->m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
		POS = CTX->m_count.m_name;
		unless((CTX->m_count.m_name = emit_opcode(
						&CTX->m_section.m_name
						, CTX->m_count.m_name
						, SpvOpName, 0)))
			goto LBL_ABRT;

		unless((CTX->m_count.m_name = aclutil_emit_wrd(
						&CTX->m_section.m_name
						, CTX->m_count.m_name
						, THIS_ENTRY_POINT.m_id)))
			goto LBL_ABRT;
		unless((CTX->m_count.m_name = aclutil_emit_str(
						&CTX->m_section.m_name
						, CTX->m_count.m_name
						, NAME.data)))
			goto LBL_ABRT;
		set_oprnd_count_for_opcode(
				aclutil_get_wrd_of_vec(&CTX->m_section.m_name)[POS]
				, CTX->m_count.m_name - POS - 1
				);
	}
	CTX->m_err = ACLSPV_COMPILE_OK;

LBL_ABRT:
	clang_disposeString(NAME);
	return THIS_ENTRY_POINT;
}
