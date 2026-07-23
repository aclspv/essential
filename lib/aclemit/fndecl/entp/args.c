#include "./args.h"

#include <ae2f/bll.h>
#include <ae2f/pat/dbg.h>

#include <util/scale.h>
#include <util/bind.h>
#include <util/cxtp.h>
#include <util/wrdemit.h>
#include <util/constant.h>
#include <util/emitx.h>
#include <util/iddef.h>

#include <aclspv.h>
#include <aclmem.h>
#include <aclcursor.h>

#include <aclattr/storage_class.h>
#include <aclattr/specid.h>
#include <aclattr/location.h>

static enum CXChildVisitResult	s_attr_set(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict h_data) {
	CXString TEXT;
	enum CXChildVisitResult RES = CXChildVisit_Break;
	const char* NEEDLE;
	unsigned SET;

	unless(h_cur.kind == CXCursor_AnnotateAttr) return CXChildVisit_Recurse;
	TEXT = clang_getCursorSpelling(h_cur);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_set")) {
		RES = CXChildVisit_Continue;
		goto LBL_FINI;
	}

	sscanf(NEEDLE, "aclspv_set ( %u )", &SET);

	*((aclspv_wrd_t* ae2f_restrict)h_data) = (aclspv_wrd_t)SET;

LBL_FINI:
	clang_disposeString(TEXT);
	return RES;
}

enum CXChildVisitResult aclemit_fndecl_entp_arg(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict h_data) {
	ae2f_unused aclspv_wrdcount_t	POS_FOR_LOCAL = 0xFFFFFFFF;
	CXString	PARAM_TY_NAME;
	CXType		PARAM_TY;
	aclutil_bind	INFO;
	struct aclemit_fndecl_entp_arg_data* ae2f_restrict DATABASE = h_data;

#define	PUSH_SIZE	DATABASE->m_push_size
#define BIND_IDX	DATABASE->m_idx_bind
#define CTX		DATABASE->m_ctx 
#define	ARG_IDX		DATABASE->m_idx_arg 
#define	INTERFACE_COUNT	DATABASE->m_num_interface
#define	IO_ARG_IDX	DATABASE->m_idx_ioarg
#define	PSH_CONSTANT_ID	DATABASE->m_id_push_constant

	ae2f_ifezerr(CTX)		return CXChildVisit_Break;
	ae2f_ifnezerr(CTX->m_err) 	return CXChildVisit_Break;

	unless(h_cur.kind == CXCursor_ParmDecl) {
		return CXChildVisit_Continue;
	}

	PARAM_TY	= clang_getCursorType(h_cur);
	ae2f_unexpected_but_if(ARG_IDX > UINT32_MAX) {
		CTX->m_err = ACLSPV_COMPILE_TOO_BIG;
		return CXChildVisit_Break;
	}

	INFO.m_unified.m_arg_idx	= (aclspv_wrd_t)ARG_IDX;
	INFO.m_unified.m_cursor		= h_cur;
	INFO.m_unified.m_entp_idx	= CTX->m_id++;

	/** not pointer|array type -> is push constant! */
	unless((PARAM_TY.kind == CXType_Pointer) | (PARAM_TY.kind == CXType_ConstantArray)) {
		INFO.m_unified.m_var_id		= (aclspv_wrd_t)PSH_CONSTANT_ID;
		INFO.m_pshconst.m_pad		= (aclspv_wrd_t)PUSH_SIZE;
		/** get type size by calculating one by one */
		PUSH_SIZE += INFO.m_pshconst.m_size
			= aclutil_sz_to_count(clang_Type_getSizeOf(PARAM_TY));

		dbg_prefix(a);
		dbg_call((void), fprintf(stderr, "PUSHINFO: %u %u %u\n"
				, INFO.m_unified.m_var_id
				, INFO.m_pshconst.m_pad
				, PUSH_SIZE));

		return CXChildVisit_Continue;
	}

	INFO.m_unified.m_var_id			= CTX->m_id++;
	INFO.m_unified.m_var_elm_type_id	= aclutil_mk_constant_struct_id(1, CTX);
	INFO.m_unified.m_var_type_id		= aclutil_mk_constant_ptr_storage_id(1, CTX);

	INFO.m_bindable.m_set			= 0;
	INFO.m_bindable.m_binding		= (aclspv_wrd_t)BIND_IDX;

	/** FETCH BIND END */
	if(CTX->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
		CXString	PARAM_NAME, FUNC_NAME;
		FUNC_NAME	= clang_getCursorSpelling(h_parent);
		PARAM_NAME	= clang_getCursorSpelling(h_cur);


		ae2f_ifnezerr_strict(aclmem_calloc(
					&CTX->m_tmp.m_v0
					, (aclspv_wrd_t)(strlen(FUNC_NAME.data) + strlen(PARAM_NAME.data) + 3)))
		{
			CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_ENTP_ARGS;
			clang_disposeString(FUNC_NAME);
			clang_disposeString(PARAM_NAME);
			return CXChildVisit_Break;
		}

		{
			char* ae2f_restrict const	
				NAME_MERGE = CTX->m_tmp.m_v0.m_p;

			const aclspv_wrdcount_t	POS = CTX->m_count.m_name;


			unless(NAME_MERGE) {
LBL_ABRT_NALLOC:
				CTX->m_err = ACLSPV_COMPILE_ALLOC_FAILED_ENTP_ARGS;
				clang_disposeString(FUNC_NAME);
				clang_disposeString(PARAM_NAME);
				return CXChildVisit_Break;
			}

			strcpy(NAME_MERGE, FUNC_NAME.data);
			strcat(NAME_MERGE, "::");
			strcat(NAME_MERGE, PARAM_NAME.data);


#define EMIT_POS	CTX->m_count.m_name
			ae2f_expected_but_else((POS_FOR_LOCAL = EMIT_POS = emit_opcode(
							&CTX->m_section.m_name
							, EMIT_POS
							, SpvOpName, 0))) 
				goto LBL_ABRT_NALLOC;
			ae2f_expected_but_else((EMIT_POS = aclutil_emit_wrd(
							&CTX->m_section.m_name
							, EMIT_POS
							, INFO.m_unified.m_var_id)))
				goto LBL_ABRT_NALLOC;
			ae2f_expected_but_else((EMIT_POS = aclutil_emit_str(
							&CTX->m_section.m_name
							, EMIT_POS
							, NAME_MERGE))) 
				goto LBL_ABRT_NALLOC;
			set_oprnd_count_for_opcode(
					aclutil_get_wrd_of_vec(&CTX->m_section.m_name)[POS]
					, EMIT_POS - POS - 1);
#undef	EMIT_POS


		}

		clang_disposeString(FUNC_NAME);
		clang_disposeString(PARAM_NAME);
	}

	INFO.m_unified.m_storage_class = SpvStorageClassMax;

	PARAM_TY_NAME	= clang_getTypeSpelling(PARAM_TY);

	/** FIXME: we are making them attributes */
	if(PARAM_TY.kind == CXType_ConstantArray)
		INFO.m_unified.m_storage_class = SpvStorageClassWorkgroup;

	/** Attriburte: storage class */
	clang_visitChildren(
			h_cur
			, s_attr_storage_class
			, &INFO.m_unified.m_storage_class
			);

	clang_disposeString(PARAM_TY_NAME);

	switch(INFO.m_unified.m_storage_class) {
		default:
		case SpvStorageClassMax:
			CTX->m_err = ACLSPV_COMPILE_STORAGE_CLASS_UNDEFINED;
			return CXChildVisit_Break;

			/** BINDABLES */
			ae2f_unexpected_but_if(0) {
				ae2f_unreachable();
				case SpvStorageClassUniform:
				INFO.m_unified.m_var_type_id = aclutil_mk_constant_ptr_uniform_id(1, CTX);
				ae2f_expected_but_else(INFO.m_unified.m_var_type_id)  {
					return CXChildVisit_Break;
				}
			}

			ae2f_fallthrough;

		case SpvStorageClassStorageBuffer:
			clang_visitChildren(h_cur
					, s_attr_set
					, &INFO.m_bindable.m_set
					);

			ae2f_ifezerr(CTX->m_count.m_decorate = aclutil_emitx_4(
						/** Decorations::Descriptorsets */
						&CTX->m_section.m_decorate
						, CTX->m_count.m_decorate
						, SpvOpDecorate
						, INFO.m_unified.m_var_id
						, SpvDecorationDescriptorSet
						, INFO.m_bindable.m_set
						)) return CXChildVisit_Break;

			ae2f_ifezerr(CTX->m_count.m_decorate = aclutil_emitx_4(
						/** Decorations::Binding */
						&CTX->m_section.m_decorate
						, CTX->m_count.m_decorate
						, SpvOpDecorate
						, INFO.m_unified.m_var_id
						, SpvDecorationBinding
						, INFO.m_bindable.m_binding
						)) return CXChildVisit_Break;

			ae2f_ifezerr(CTX->m_count.m_vars = util_emitx_variable(
						&CTX->m_section.m_vars
						, CTX->m_count.m_vars
						, INFO.m_unified.m_var_type_id
						, INFO.m_unified.m_var_id
						, INFO.m_unified.m_storage_class
						)) return CXChildVisit_Break;

			++BIND_IDX;
			break;

		case SpvStorageClassWorkgroup:
			PARAM_TY_NAME	= clang_getTypeSpelling(PARAM_TY);
			unless(strstr(PARAM_TY_NAME.data, "local")) {
				clang_disposeString(PARAM_TY_NAME);
				CTX->m_err = ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS;
				return CXChildVisit_Break;
			} clang_disposeString(PARAM_TY_NAME);

			unless(PARAM_TY.kind == CXType_ConstantArray) {
				CTX->m_err = ACLSPV_COMPILE_WORKGROUP_MUST_BE_CONSTANT_ARRAY;
				return CXChildVisit_Break;
			}

			{
				aclspv_wrd_t	SPECID_WORK	= 0xFFFFFFFF;
				const intmax_t NUM_ARR_ORIG	= clang_getArraySize(PARAM_TY);
				const intmax_t REAL_ARR_ORIG	= clang_Type_getSizeOf(PARAM_TY);
				const aclspv_wrd_t NUM_ARR	= NUM_ARR_ORIG < 0 || NUM_ARR_ORIG > UINT32_MAX ? 0 : (aclspv_wrd_t)NUM_ARR_ORIG;
				const aclspv_wrd_t REAL_ARR	= REAL_ARR_ORIG < 0 || REAL_ARR_ORIG > UINT32_MAX ? 0 : (aclspv_wrd_t)REAL_ARR_ORIG;
				const aclspv_wrd_t ELM_ARR	= ae2f_expected(NUM_ARR && REAL_ARR) ? REAL_ARR / NUM_ARR : 0;
				const aclspv_id_t VAL_ELM	= aclutil_mk_constant_val_id(ELM_ARR, CTX);
				const aclspv_id_t VAL_2		= aclutil_mk_constant_val_id(2, CTX);
				const aclspv_id_t VAL_3		= aclutil_mk_constant_val_id(3, CTX);

				assert(REAL_ARR >= NUM_ARR && !(REAL_ARR % NUM_ARR));
				{ ae2f_assume(REAL_ARR >= NUM_ARR && !(REAL_ARR % NUM_ARR)); }

				ae2f_expected_but_else(VAL_2)		return CXChildVisit_Break;
				ae2f_expected_but_else(VAL_ELM)		return CXChildVisit_Break;

				clang_visitChildren(
						h_cur
						, s_attr_specid 
						, &SPECID_WORK
						);

				ae2f_expected_but_else(NUM_ARR) {
					CTX->m_err = ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS;
					return CXChildVisit_Break;
				}

				ae2f_expected_if(SPECID_WORK != 0xFFFFFFFF) {
					const aclspv_id_t SPECIDSPIRV_ 
						= aclutil_mk_constant_spec_id(SPECID_WORK, NUM_ARR, CTX);

					const aclspv_id_t SPECIDSPIRV
						= SPECIDSPIRV_ ? SPECIDSPIRV_ : 
						aclutil_mk_constant_spec64_id(
								SPECID_WORK
								, NUM_ARR
								, CTX
								);

					ae2f_expected_but_else(SPECIDSPIRV) {
						CTX->m_err = ACLSPV_COMPILE_MET_INVAL_ENTP_ARGS;
						return CXChildVisit_Break; 
					}

					INFO.m_work.m_arr_count_id = SPECIDSPIRV;
				} else {
					INFO.m_work.m_arr_count_id = aclutil_mk_constant_val_id(NUM_ARR, CTX);
				}


				ae2f_expected_but_else(CTX->m_count.m_types = util_emitx_spec_constant_op2(
							&CTX->m_section.m_types
							, CTX->m_count.m_types
							, ACLSPV_ID_I32
							, CTX->m_id
							, SpvOpIMul
							, CTX->m_id - 1
							, VAL_ELM
							))
					return CXChildVisit_Break;

				ae2f_expected_but_else(CTX->m_count.m_types = util_emitx_spec_constant_op2(
							&CTX->m_section.m_types
							, CTX->m_count.m_types
							, ACLSPV_ID_I32
							, CTX->m_id + 1
							, SpvOpIAdd
							, CTX->m_id
							, VAL_3
							))
					return CXChildVisit_Break;

				ae2f_expected_but_else(CTX->m_count.m_types = util_emitx_spec_constant_op2(
							&CTX->m_section.m_types
							, CTX->m_count.m_types
							, ACLSPV_ID_I32
							, CTX->m_id + 2
							, SpvOpShiftRightLogical
							, CTX->m_id + 1
							, VAL_2
							)) return CXChildVisit_Break;

				ae2f_expected_but_else(CTX->m_count.m_types = util_emitx_type_array(
							&CTX->m_section.m_types
							, CTX->m_count.m_types
							, CTX->m_id + 3
							, ACLSPV_ID_I32
							, CTX->m_id + 2
							)) return CXChildVisit_Break;

				ae2f_expected_but_else(CTX->m_count.m_types = util_emitx_type_pointer(
							&CTX->m_section.m_types
							, CTX->m_count.m_types
							, CTX->m_id + 4
							, SpvStorageClassWorkgroup /* c_storage_class */
							, CTX->m_id + 3
							)) return CXChildVisit_Break;

				INFO.m_unified.m_var_id		= CTX->m_id + 5;
				INFO.m_unified.m_var_type_id		= CTX->m_id + 4;
				INFO.m_unified.m_var_elm_type_id	= CTX->m_id + 3;
				INFO.m_unified.m_storage_class		= SpvStorageClassWorkgroup;

				CTX->m_id	+= 6;
				CTX->m_err	 = ACLSPV_COMPILE_OK;

				if(CTX->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
					((aclspv_wrd_t* ae2f_restrict)CTX->m_section.m_name.m_p)[POS_FOR_LOCAL]
						= INFO.m_unified.m_var_id;
				}

				ae2f_expected_but_else(CTX->m_count.m_vars = util_emitx_variable(
							&CTX->m_section.m_vars
							, CTX->m_count.m_vars
							, INFO.m_unified.m_var_type_id
							, INFO.m_unified.m_var_id
							, SpvStorageClassWorkgroup
							)) return CXChildVisit_Break;
			}

			break;

			/** LOCATED */

			ae2f_ifnezerr_strict(0) {
				ae2f_unreachable();

				/** TODO: those are hard-coded. Should be dynamicly loaded. */
				case SpvStorageClassInput:
				case SpvStorageClassOutput:
				{
					const aclspv_id_t I32 = aclid_mk_default_id(ACLSPV_ID_I32, CTX);
					const aclspv_id_t I32V4 = aclutil_mk_cxtp_vec(
							ACLSPV_ID_I32
							, 4, CTX);

					ae2f_expected_but_else(I32V4 && I32) {
						return CXChildVisit_Break;
					}

					INFO.m_unified.m_var_type_id = aclutil_mk_cxtp_ptr(
							I32V4
							, (enum SpvStorageClass_)INFO.m_unified.m_storage_class
							, CTX
							);
				}
			}

			INFO.m_io.m_location = 0xFFFFFFFF;
			clang_visitChildren(h_cur, s_attr_location, &INFO.m_io.m_location);
			if(INFO.m_io.m_location == 0xFFFFFFFF)
				INFO.m_io.m_location = 0;

#define		THIS_ENTP	DATABASE->m_entp
			/** TODO: this will now uintvec4. make this flexible some day */

			INFO.m_unified.m_var_id			= CTX->m_id++;
			INFO.m_unified.m_var_elm_type_id	= aclutil_mk_constant_vec32_id(4, CTX);

			ae2f_expected_but_else(CTX->m_count.m_decorate = aclutil_emitx_4(
						&CTX->m_section.m_decorate
						, CTX->m_count.m_decorate
						, SpvOpDecorate
						, INFO.m_unified.m_var_id
						, SpvDecorationLocation
						, INFO.m_io.m_location
						)) return CXChildVisit_Break;

			ae2f_expected_but_else(CTX->m_count.m_vars = util_emitx_variable(
						&CTX->m_section.m_vars
						, CTX->m_count.m_vars
						, INFO.m_unified.m_var_type_id
						, INFO.m_unified.m_var_id
						, INFO.m_unified.m_storage_class
						)) return CXChildVisit_Break;

			if(CTX->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
				aclutil_get_wrd_of_vec(&CTX->m_section.m_name)[POS_FOR_LOCAL] = INFO.m_unified.m_var_id;
			}

			ae2f_ifnezerr_strict(s_aclmem_cgrow2(
					&CTX->m_section.m_entp
					, CTX->m_count.m_entp + 1
					, sizeof(aclspv_wrd_t)
					, ACLSPV_COMPILE_ALLOC_FAILED_ENTP_ARGS))
				return CXChildVisit_Break;

			((aclspv_wrd_t* ae2f_restrict)CTX->m_section.m_entp.m_p)[CTX->m_count.m_entp++]
				= INFO.m_unified.m_var_id;

			((aclspv_wrd_t* ae2f_restrict)CTX->m_section.m_entp.m_p)[DATABASE->m_idx_entp_buf_count]
				+= 0x10000;

#undef		THIS_ENTP

			break;
	}

	CTX->m_err = ACLSPV_COMPILE_OK;

	{
		const aclspv_wrd_t CURSOR_IDX = aclutil_mk_cursor_base(
				CTX->m_num_cursor
				, &CTX->m_cursors
				, INFO.m_unified.m_cursor
				);

		ae2f_unexpected_but_if(CURSOR_IDX == CTX->m_num_cursor + 1) {
			CTX->m_err = (enum ACLSPV_COMPILE_)ae2f_bll_sel_bool_naive1(
					uint_fastenum_t
					, CTX->m_err
					, (uint_fastenum_t) CTX-> m_err
					, ACLSPV_COMPILE_UNHANDLED
					);

			return CXChildVisit_Break;
		}

		dbg_prefix();
		dbg_call((void), fprintf(stderr
					, "fndecl::entp::args::decl %u\n"
					, CURSOR_IDX));

		CTX->m_num_cursor
			= CURSOR_IDX == CTX->m_num_cursor;

		CURSOR_IDX[(x_aclcursor* ae2f_restrict)CTX->m_cursors.m_p]
			.m_cursor = INFO.m_unified.m_cursor;

		CURSOR_IDX[(x_aclcursor* ae2f_restrict)CTX->m_cursors.m_p]
			.m_kind = ACLCURSOR_PARENTP;

		CURSOR_IDX[(x_aclcursor* ae2f_restrict)CTX->m_cursors.m_p]
			.m_data.m_prm_decl.m_info = INFO;
	}

	++ARG_IDX;
	++INTERFACE_COUNT;
	return CXChildVisit_Continue;


#undef	PUSH_SIZE
#undef	BIND_IDX
#undef	CTX
#undef	ARG_IDX
#undef	INTERFACE_COUNT
#undef	VAR_INFOS
#undef	DATABASE
}
