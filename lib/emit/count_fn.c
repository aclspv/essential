#include "./count_fn.h"
#include "aclspv.h"

#include <attr/storage_class.h>
#include <attr/execmodel.h>
#include <attr/wrkgroup.h>

#include <assert.h>

#include <clang-c/Index.h>
#include <util/ctx.h>
#include <util/emitx.h>
#include <util/is_kernel.h>
#include <util/fn.h>
#include <util/entp.h>
#include <util/constant.h>
#include <util/iddef.h>
#include <util/log.h>

#include <spirv/unified1/spirv.h>

ACLSPV_ABI_IMPL enum CXChildVisitResult aclemit_iter_entry_point(CXCursor h_cur, CXCursor h_parent, CXClientData CTX) {
#define CTX	((x_aclspv_ctx* ae2f_restrict)CTX)
	ae2f_ifnezerr(CTX->m_err) return CXChildVisit_Break;

	unless(h_cur.kind == CXCursor_CompoundStmt && h_parent.kind == CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	if(util_is_kernel(h_parent)) {
		const CXString NAME = clang_getCursorSpelling(h_parent);
		struct	aclattr_wrkgroup_prm	XYZ;
		enum SpvExecutionModel_	EXEC_MODEL = SpvExecutionModelMax;

		aclspv_wrdcount_t	POS = CTX->m_count.m_entp;
		aclspv_wrd_t		IOCOUNT = 0;
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
				util_constant* nd = aclutil_mk_constant_node(XYZ.m_##K_idx, CTX);	\
				ae2f_ifezerr(nd) goto LBL_ABRT;						\
				if(nd->m_const_spec_id) {						\
					ae2f_ifezerr_strict(aclid_is_int(nd->m_const_spec_type)) {	\
						CTX->m_err = ACLSPV_COMPILE_WORKGROUP_SIZE_NO_INT;	\
						goto LBL_ABRT;						\
					}								\
					XYZ.m_##K_idx	= nd->m_const_spec_id;				\
				} else {								\
					ae2f_ifezerr(XYZ.m_##K_idx = aclutil_mk_constant_spec_id(	\
								XYZ.m_##K_idx, 1, CTX)) {		\
						CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED;		\
						goto LBL_ABRT;						\
					}								\
				}									\
			} else ae2f_ifezerr(aclutil_mk_constant_val_id(	\
						(uint_least64_t)XYZ.m_0	\
						, CTX)) {	\
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED;	\
				goto LBL_ABRT;	\
			}

			if(XYZ.m_flag_specid) {
				ae2f_ifnezerr(CTX->m_spirv_ver < aclspv_ver_spv(1, 2)) {
					CTX->m_err = ACLSPV_COMPILE_V_WRKGROUP_SIZE;
					err_puts("workgroup size id is for spirv 1.2");
					goto LBL_ABRT;
				}
					
				S_GETSPECID(0);
				S_GETSPECID(1);
				S_GETSPECID(2);
			}
#undef	S_GETSPECID
		}

		{
			int ARGI	= clang_Cursor_getNumArguments(h_parent);
			while(ARGI --> 0) {	/** All-Capturing Iterating Annonymous Routine Syntax (ACIARS) */
				CXCursor ARG	= clang_Cursor_getArgument(h_parent, (unsigned)ARGI);
				aclspv_wrd_t	STORAGE = SpvStorageClassMax;
				clang_visitChildren(ARG, s_attr_storage_class, &STORAGE);

				if(STORAGE == SpvStorageClassInput || STORAGE == SpvStorageClassOutput) {
					++IOCOUNT;
				}
			}
		}


#define THIS_ENTRY_POINT	((util_entp_t* ae2f_restrict)CTX->m_fnlist.m_entp.m_p)[CTX->m_tmp.m_w0]
		THIS_ENTRY_POINT.m_fn	= h_parent;
		THIS_ENTRY_POINT.m_id	= CTX->m_tmp.m_w3 + CTX->m_tmp.m_w0;

		if(IOCOUNT) {
			THIS_ENTRY_POINT.m_io.m_anchour = CTX->m_id;
			THIS_ENTRY_POINT.m_io.m_num	= IOCOUNT;
			CTX->m_id += IOCOUNT;
		}

		/** OpEntryPoint */
		unless((CTX->m_count.m_entp = emit_opcode(
						&CTX->m_section.m_entp
						, CTX->m_count.m_entp
						, SpvOpEntryPoint, 0)))
			goto LBL_ABRT;
		unless((CTX->m_count.m_entp = util_emit_wrd(
						&CTX->m_section.m_entp
						, CTX->m_count.m_entp
						, EXEC_MODEL)))
			goto LBL_ABRT;
		unless((CTX->m_count.m_entp = util_emit_wrd(
						&CTX->m_section.m_entp
						, CTX->m_count.m_entp
						, THIS_ENTRY_POINT.m_id)))
			goto LBL_ABRT;
		unless((CTX->m_count.m_entp = util_emit_str(
						&CTX->m_section.m_entp
						, CTX->m_count.m_entp
						, NAME.data))) 
			goto LBL_ABRT;

		if(IOCOUNT) {
			aclspv_wrd_t	IO_IDX = 0;
			while(IO_IDX < IOCOUNT) {
				unless((CTX->m_count.m_entp = util_emit_wrd(
								&CTX->m_section.m_entp
								, CTX->m_count.m_entp
								, THIS_ENTRY_POINT.m_io.m_anchour + IO_IDX)))
					goto LBL_ABRT;
				++IO_IDX;
			}
		}

		set_oprnd_count_for_opcode(
				get_wrd_of_vec(&CTX->m_section.m_entp)[POS]
				, CTX->m_count.m_entp - POS - 1
				);

		/** OpExecMode */
		if(EXEC_MODEL == SpvExecutionModelGLCompute || EXEC_MODEL == SpvExecutionModelKernel)
			ae2f_expected_but_else(CTX->m_count.m_execmode = util_emitx_6(
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

			unless((CTX->m_count.m_name = util_emit_wrd(
							&CTX->m_section.m_name
							, CTX->m_count.m_name
							, THIS_ENTRY_POINT.m_id)))
				goto LBL_ABRT;
			unless((CTX->m_count.m_name = util_emit_str(
							&CTX->m_section.m_name
							, CTX->m_count.m_name
							, NAME.data)))	
				goto LBL_ABRT;
			set_oprnd_count_for_opcode(
					get_wrd_of_vec(&CTX->m_section.m_name)[POS]
					, CTX->m_count.m_name - POS - 1
					);

		}
		CTX->m_err = ACLSPV_COMPILE_OK;
		++CTX->m_tmp.m_w0;

		clang_disposeString(NAME);
		return CXChildVisit_Continue;
		ae2f_unreachable();

LBL_ABRT:
		clang_disposeString(NAME);
		return CXChildVisit_Break;
	} else {
#define	THIS_FUNCTION	((util_fn_t* ae2f_restrict)CTX->m_fnlist.m_fn.m_p)[CTX->m_tmp.m_w1]
		THIS_FUNCTION.m_fn = h_parent;
		THIS_FUNCTION.m_id = CTX->m_id++;
	}
#undef	CTX
#undef	THIS_ENTRY_POINT

	return CXChildVisit_Continue;
}

ACLSPV_ABI_IMPL enum CXChildVisitResult aclemit_count_fn(CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx) {
#define CTX	((x_aclspv_ctx* ae2f_restrict)h_ctx)
	assert(CTX);

	unless(h_cur.kind == CXCursor_CompoundStmt && h_parent.kind == CXCursor_FunctionDecl)
		return CXChildVisit_Recurse;

	if(util_is_kernel(h_parent)) {
		++CTX->m_fnlist.m_num_entp;
	} else {
		++CTX->m_fnlist.m_num_fn;
	}

	return CXChildVisit_Continue;
#undef	CTX
}
