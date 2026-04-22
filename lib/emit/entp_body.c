#include "./entp_body.h"
#include "./expr.h"

#include <aclspv.h>

#include <ae2f/Keys.h>
#include <ae2f/c90/StdInt.h>
#include <ae2f/c90/StdBool.h>

#include <clang-c/CXString.h>

#include <clang-c/Index.h>
#include <util/fnctx.h>
#include <util/emitx.h>
#include <util/iddef.h>
#include <util/constant.h>
#include <util/cursor.h>
#include <util/scale.h>
#include <util/literal.h>
#include <util/u32_to_hex8.auto.h>
#include <util/cxtp.h>
#include <util/log.h>

#include <attr/specid.h>
#include <attr/btcast.h>
#include <attr/ifpart.h>

#include <spirv/unified1/spirv.h>


#define	___mkname_on_dbg(c_ID)	if(CTX->m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {	\
	const aclspv_wrd_t POS	= CTX->m_count.m_name;				\
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free				\
			, CTX->m_tmp.m_v0, strlen(SPELL.data) + 10		\
			);							\
	ae2f_expected_but_else(CTX->m_tmp.m_v0.m_p)				\
	goto LBL_FAIL;								\
	((char* ae2f_restrict)CTX->m_tmp.m_v0.m_p)[8] = ':';			\
	((char* ae2f_restrict)CTX->m_tmp.m_v0.m_p)[9] = '\0';			\
	_util_u32_to_hex8((c_ID)			\
			, ((char* ae2f_restrict)CTX->m_tmp.m_v0.m_p));	\
	strcat(CTX->m_tmp.m_v0.m_p, SPELL.data);			\
	ae2f_expected_but_else((CTX->m_count.m_name = emit_opcode(	\
					&CTX->m_section.m_name		\
					, CTX->m_count.m_name	\
					, SpvOpName, 0))			\
					) goto LBL_FAIL;					\
	ae2f_expected_but_else((CTX->m_count.m_name = util_emit_wrd(		\
					&CTX->m_section.m_name		\
					, CTX->m_count.m_name	\
					, c_ID	\
					)))						\
	goto LBL_FAIL;								\
	ae2f_expected_but_else((CTX->m_count.m_name = util_emit_str(			\
					&CTX->m_section.m_name			\
					, CTX->m_count.m_name		\
					, CTX->m_tmp.m_v0.m_p 			\
					)))						\
	goto LBL_FAIL;								\
	set_oprnd_count_for_opcode(get_wrd_of_vec(					\
				&CTX->m_section.m_name)[POS]				\
				, CTX->m_count.m_name - POS - 1);				\
}

enum CXChildVisitResult aclemit_entp_body(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData rdwr_data) {
#define	FNCTX		((aclutil_fnctx* ae2f_restrict) rdwr_data)
#define	CTX		FNCTX->m_ctx
#define	jmpfail(c_why)	{ (CTX)->m_err = (c_why); goto LBL_FAIL; }
	const enum CXCursorKind KIND	= (h_cur).kind;
	const CXString KINDSPELL	= clang_getCursorKindSpelling(KIND);
	const CXString SPELL		= clang_getCursorSpelling(h_cur);

	ae2f_expected_but_else(KIND)	{
		CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
		goto LBL_FAIL;
	}

	FNCTX->m_state0 |= (uint_leastenum_t)(KIND != CXCursor_LabelStmt && KIND != CXCursor_ReturnStmt) << (uint_leastenum_t)(ACLUTIL_FNCTX_STATEPAD0_LBL_DO_SOMETHING);
	FNCTX->m_state0 &= ~((uint_leastenum_t)(KIND != CXCursor_ReturnStmt) << (uint_leastenum_t)(ACLUTIL_FNCTX_STATEPAD0_FNRET));

	switch((uintmax_t)KIND) {
		case CXCursor_VarDecl:
			{
				aclspv_wrdcount_t IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, h_cur);

				aclspv_id_t	TYPE_ID = 0;

				const CXType	TYPE = clang_getCanonicalType(clang_getCursorType(h_cur));

				ae2f_unexpected_but_if(IDX == CTX->m_num_cursor + 1)
					goto LBL_FAIL;

#define	CURSOR	((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]

				if(IDX == CTX->m_num_cursor)
					++CTX->m_num_cursor;

				switch((uintmax_t)TYPE.kind) {
					case CXType_UInt:
					case CXType_UShort:
					case CXType_UChar:
						ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_U32, CTX))
							goto LBL_FAIL;


						TYPE_ID = aclutil_mk_cxtp_ptr(
								ID_DEFAULT_U32
								, SpvStorageClassFunction
								, CTX
								);

						CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_U32;


						ae2f_unexpected_but_if(0) {
							ae2f_unreachable();
							case CXType_Int:
							case CXType_Short:
							case CXType_SChar:
							ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_I32, CTX))
								goto LBL_FAIL;
							TYPE_ID = aclutil_mk_cxtp_ptr(
									ID_DEFAULT_I32
									, SpvStorageClassFunction
									, CTX
									);

							CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_I32;
						}

						CURSOR.m_data.m_var_simple.m_fits_32bit = 1;

						ae2f_unexpected_but_if(0)
						{
							ae2f_unreachable();
							case CXType_ULongLong:
							case CXType_ULong:
							ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_U64, CTX))
								goto LBL_FAIL;
							TYPE_ID = aclutil_mk_cxtp_ptr(
									ID_DEFAULT_U64
									, SpvStorageClassFunction
									, CTX
									);

							CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_U64;
						}

						ae2f_unexpected_but_if(0)
						{
							ae2f_unreachable();
							case CXType_LongLong:
							case CXType_Long:
							ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_I64, CTX))
								goto LBL_FAIL;
							TYPE_ID = aclutil_mk_cxtp_ptr(
									ID_DEFAULT_I64
									, SpvStorageClassFunction
									, CTX
									);

							CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_I64;
						}

						CURSOR.m_data.m_var_simple.m_is_integer = 1;
						CURSOR.m_data.m_var_simple.m_fits_64bit = 1;

						break;


						ae2f_unexpected_but_if(0) {
							ae2f_unreachable();
							case CXType_BFloat16:
							case CXType_Float16:
							ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_F16, CTX))
								goto LBL_FAIL;
							TYPE_ID = aclutil_mk_cxtp_ptr(
									ID_DEFAULT_F16
									, SpvStorageClassFunction
									, CTX
									);

							CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_F16;
						}

						ae2f_unexpected_but_if(0) {
							ae2f_unreachable();
							case CXType_Float:
							ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_F32, CTX))
								goto LBL_FAIL;
							TYPE_ID = aclutil_mk_cxtp_ptr(
									ID_DEFAULT_F32
									, SpvStorageClassFunction
									, CTX
									);

							CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_F32;
						}

						CURSOR.m_data.m_var_simple.m_fits_32bit = 1;

						ae2f_unexpected_but_if(0) {
							ae2f_unreachable();
							case CXType_Double:
							ae2f_expected_but_else(aclid_mk_default_id(ID_DEFAULT_F64, CTX))
								goto LBL_FAIL;
							TYPE_ID = aclutil_mk_cxtp_ptr(
									ID_DEFAULT_F64
									, SpvStorageClassFunction
									, CTX
									);
							CURSOR.m_data.m_var_simple.m_type_id = ID_DEFAULT_F64;
						}
						CURSOR.m_data.m_var_simple.m_fits_64bit = 1;
						CURSOR.m_data.m_var_simple.m_is_predictable = 0;

						break;

						/** complex types. we make it later. */

					case CXType_Record:
						{
							const intmax_t	TY_SIZEOF_ = clang_Type_getSizeOf(TYPE);
							const uintmax_t	 TY_SIZEOF = TY_SIZEOF_ < 0 ? 0 : (uintmax_t)TY_SIZEOF_;
							const aclspv_wrd_t	 TY_SIZE_WRD = TY_SIZEOF < UINT32_MAX 
								? (aclspv_wrd_t)TY_SIZEOF : 0;

							ae2f_expected_but_else(TY_SIZE_WRD)
								jmpfail(ACLSPV_COMPILE_MET_INVAL);

							ae2f_expected_but_else(TYPE_ID = aclutil_mk_constant_ptr_func(
										(TY_SIZE_WRD + 3) >> 2
										, CTX))
								goto LBL_FAIL;
							CURSOR.m_data.m_var_simple.m_type_id = aclutil_mk_constant_structpriv_id(
									(TY_SIZE_WRD + 3) >> 2, CTX
									);
						}
						break;

					default:
						{
							CXString NAME = clang_getTypeSpelling(TYPE);
							err_prefix(a);
							err_call(
									fprintf, (
										stderr
										, "Met the unknown type(%u): %s\n"
										, TYPE.kind
										, (const char*)NAME.data
										);
								)
								clang_disposeString(NAME);
						}
						break;
				}

				ae2f_expected_but_else(TYPE_ID)
					goto LBL_FAIL;

				CURSOR.m_data.m_var_simple.m_ptr_type_id = TYPE_ID;
				CURSOR.m_data.m_var_simple.m_id = CTX->m_id++;
				CURSOR.m_data.m_var_simple.m_is_predictable = 0;
				CURSOR.m_data.m_var_simple.m_mask_literal = 0;

				/**
				 * TODO:
				 * So far I have defined the variable but not with the initialiser.
				 * Make initialising part if possible.
				 * */
				if(aclid_is_arithmetic(CURSOR.m_data.m_var_simple.m_type_id))
				{
					CXCursor	VAR_INIT = clang_Cursor_getVarDeclInitializer(h_cur);

#if 1
					aclspv_id_t	SPECID = 0xFFFFFFFF;

					clang_visitChildren(h_cur, s_attr_specid, &SPECID);

					if(SPECID != 0xFFFFFFFF) {
						/** specid! */
						util_constant* ae2f_restrict const NODE
							= aclutil_mk_constant_node(SPECID, CTX);

#if 1 
						dbg_prefix(i);
						dbg_call(fprintf, (stderr, "Spec ID detected: %u\n", SPECID));
#endif

						if(NODE && NODE->m_const_spec_id) {
							dbg_prefix(a);
							dbg_call(fprintf, (stderr, "Spec ID definition spotted.: %u\n"
										, NODE->m_const_spec_id));

							switch(aclexpr_arithmetic_cast_non_literal(
										NODE->m_const_spec_id
										, NODE->m_const_spec_type
										, UTIL_LITERAL_CONSTANT
										, &CURSOR.m_data.m_var_simple.m_init_val
										, CURSOR.m_data.m_var_simple.m_type_id
										, CTX
										))
							{
								case EMIT_EXPR_SUCCESS_LITERAL:
								default:
									assert(0 && "unreachable & default");
									ae2f_unreachable();
									break;
								case EMIT_EXPR_NOT_THE_CASE:
									assert(0 && "not the case");
									ae2f_unreachable();
									break;

								case EMIT_EXPR_FAILURE:
									goto LBL_FAIL;
									ae2f_unexpected_but_if(0) {
										ae2f_unreachable();
										case EMIT_EXPR_SUCCESS:
										CURSOR.m_data.m_var_simple.m_mask_literal
											= UTIL_LITERAL_RT;
									}

									ae2f_unexpected_but_if(0) {
										ae2f_unreachable();
										case EMIT_EXPR_SUCCESS_CONSTANT:
										CURSOR.m_data.m_var_simple.m_mask_literal 
											= UTIL_LITERAL_CONSTANT;
									}

									CURSOR.m_data.m_var_simple.m_is_predictable = 1;
							}
						}


						goto LBL_DONE;
					}
#endif

					unless(clang_Cursor_isNull(VAR_INIT)) {
						fn_aclspv_get_expr_t*	FN_GETEXPR = aclspv_get_expr;

						{
							int IS_BTCAST = 0;
							clang_visitChildren(h_cur, s_attr_btcast, &IS_BTCAST);
							if(IS_BTCAST) FN_GETEXPR = aclspv_get_expr_btcast;
						}

						dbg_puts(("Variable has constructor."));

						switch(FN_GETEXPR(
									&CURSOR.m_data.m_var_simple.m_init_val
									, CURSOR.m_data.m_var_simple.m_type_id
									, VAR_INIT
									, &CURSOR.m_data.m_var_simple.m_literal
									, CTX))
						{
							case EMIT_EXPR_NOT_THE_CASE:
								CTX->m_err = ACLSPV_COMPILE_NO_IMPL;
								break;

							default:
							case EMIT_EXPR_FAILURE:
								CTX->m_err = ACLSPV_COMPILE_MET_INVAL;
								break;

							case EMIT_EXPR_SUCCESS_LITERAL:
								CURSOR.m_data.m_var_simple.m_mask_literal
									|= UTIL_LITERAL_MASK_LITERAL;

								dbg_puts(("Constructor is literal."));
								ae2f_fallthrough;
							case EMIT_EXPR_SUCCESS_CONSTANT:

								dbg_puts(("Constructor may be constant."));
								CURSOR.m_data.m_var_simple.m_mask_literal 
									|= UTIL_LITERAL_CONSTANT;
								ae2f_fallthrough;
							case EMIT_EXPR_SUCCESS:
								CURSOR.m_data.m_var_simple.m_is_predictable = 1;
								break;
						}
					}
				}

#undef	CURSOR

			}
			goto LBL_DONE;

		case CXCursor_ReturnStmt:
			if(FNCTX->m_state0 & (uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING)) {
				aclutil_tell_cursor_lbl(CTX->m_num_cursor, CTX->m_cursors.m_p
						, CTX);
			} else {
				CTX->m_count.m_fnimpl = FNCTX->m_lbl_snapshot;
			}

			ae2f_expected_but_else(CTX->m_count.m_fnimpl = emit_opcode(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpReturn, 0
						)) goto LBL_FAIL;
			FNCTX->m_state0 |= ACLUTIL_FNCTX_STATE0_FNRET;
			FNCTX->m_lbl_snapshot = CTX->m_count.m_fnimpl;

			if(FNCTX->m_retid != ID_DEFAULT_VOID) {
				/** have to return a value */
			}

			FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);

			/** just for branch */
			ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpLabel
						, CTX->m_id++
						)) goto LBL_FAIL;

			goto LBL_DONE;

		case CXCursor_LabelStmt:
			unless(FNCTX->m_state0 & (uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING)) {
				CTX->m_count.m_fnimpl = FNCTX->m_lbl_snapshot;
			}

			ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
						CTX->m_num_cursor
						, CTX->m_cursors.m_p, CTX))
				goto LBL_FAIL;
			{

#define	CURSOR	((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
				const aclspv_wrd_t IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, h_cur);


				ae2f_unexpected_but_if(CTX->m_num_cursor + 1 == IDX)
					goto LBL_FAIL;

				if(CTX->m_num_cursor == IDX) {
					CURSOR.m_data.m_goto_lbl.m_id = CTX->m_id++;
					++CTX->m_num_cursor;
				}

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpBranch
							, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
							.m_data.m_goto_lbl.m_id
							)) goto LBL_FAIL;

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpLabel
							, CURSOR.m_data.m_goto_lbl.m_id
							)) goto LBL_FAIL;

				___mkname_on_dbg(CURSOR.m_data.m_goto_lbl.m_id);
#undef	CURSOR
			} goto LBL_DONE;
		case CXCursor_GotoStmt:
			{
				CXCursor LBL_STMT = clang_getCursorReferenced(h_cur);
				aclspv_wrdcount_t IDX;
				assert(LBL_STMT.kind == CXCursor_LabelStmt);
				ae2f_expected_but_else(LBL_STMT.kind == CXCursor_LabelStmt)
					jmpfail(ACLSPV_COMPILE_MET_INVAL);

				IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, LBL_STMT
						);

				ae2f_unexpected_but_if(CTX->m_num_cursor + 1 == IDX) goto LBL_FAIL;
				if(CTX->m_num_cursor == IDX) {
					((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_cursor
						= LBL_STMT;
					((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX].m_data.m_goto_lbl.m_id
						= CTX->m_id++;

					++CTX->m_num_cursor;
				}


				/** snapshot */
				FNCTX->m_lbl_snapshot	= CTX->m_count.m_fnimpl;

				if(FNCTX->m_state0 & (uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING)) {
					aclutil_tell_cursor_lbl(CTX->m_num_cursor, CTX->m_cursors.m_p
							, CTX);
				}

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpBranch
							, ((util_cursor* ae2f_restrict)CTX->m_cursors.m_p)[IDX]
							.m_data.m_goto_lbl.m_id
							)) goto LBL_FAIL;

				/** just for branch */
				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpLabel
							, CTX->m_id++
							)) goto LBL_FAIL;

				FNCTX->m_state0 &= ~(uint_leastenum_t)(ACLUTIL_FNCTX_STATE0_LBL_DO_SOMETHING);
			}
			goto LBL_DONE;

			/** Literals in function scope does literally nothing in my opinion */
			goto LBL_DONE;


			/** 
			 * if this is surely just {},
			 * 	then it is just another block ready to be merged. 
			 * */
		case CXCursor_CompoundStmt:
			goto LBL_RECURSE;

		case CXCursor_BinaryOperator:
		case CXCursor_CompoundAssignOperator:
#if 1
			{
				const aclspv_wrdcount_t EXPR_IDX = aclutil_mk_cursor_base(
						CTX->m_num_cursor
						, &CTX->m_cursors
						, h_cur
						);

				aclspv_id_t	CONDVALID = CTX->m_id++;

				ae2f_expected_but_else(EXPR_IDX != CTX->m_num_cursor + 1)
					goto LBL_FAIL;

				if(EXPR_IDX == CTX->m_num_cursor)
					++CTX->m_num_cursor;

				unless(aclspv_get_expr(
							&CONDVALID
							, 0
							, h_cur
							, 0
							, CTX))
					goto LBL_FAIL;
			}
			goto LBL_DONE;
#endif
		case CXCursor_IfStmt:
			{
				struct s_attr_ifpart_t	IFPART;
				aclspv_id_t		IDVAL = 0;

				IFPART.m_n				= 0;
				IFPART.m_b_true				= CTX->m_id++;
				IFPART.m_b_false = IFPART.m_b_default	= CTX->m_id++;

				clang_visitChildren(h_cur, s_attr_ifpart, &IFPART);
				dbg_prefix(a);
				dbg_call(fprintf, (stderr, "number ifpart: %u\n", IFPART.m_n));


				dbg_prefix(a);
				dbg_call(fprintf, (stderr, "ifpart: %u %u %u\n", IFPART.m_b_true, IFPART.m_b_false, IFPART.m_b_default));

				switch(aclspv_get_expr(
							&IDVAL
							, ID_DEFAULT_BOOL
							, IFPART.m_parts[0]
							, 0, CTX))
				{
					case EMIT_EXPR_NOT_THE_CASE:
					case EMIT_EXPR_FAILURE:
					default:
						goto LBL_FAIL;

					case EMIT_EXPR_SUCCESS:
					case EMIT_EXPR_SUCCESS_CONSTANT:
					case EMIT_EXPR_SUCCESS_LITERAL:
						break;
				}

				ae2f_expected_but_else(IDVAL) {
					CTX->m_err = ACLSPV_COMPILE_IF_COND_INVAL;
					goto LBL_FAIL;
				}

				ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
							CTX->m_num_cursor
							, CTX->m_cursors.m_p
							, CTX)) goto LBL_FAIL;

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_3(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpSelectionMerge
							, IFPART.m_b_default
							, 0	/** attribute */
							)) goto LBL_FAIL;

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_4(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpBranchConditional
							, IDVAL
							, IFPART.m_b_true
							, IFPART.m_b_false
							)) goto LBL_FAIL;

				{
					aclutil_fnctx		FNCTX_NEW = *FNCTX;
					FNCTX_NEW.m_state0 = 0;

					ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
								&CTX->m_section.m_fnimpl
								, CTX->m_count.m_fnimpl
								, SpvOpLabel
								, IFPART.m_b_true
								)) goto LBL_FAIL;

					clang_visitChildren(
							IFPART.m_parts[1]
							, aclemit_entp_body
							, &FNCTX_NEW
							); ae2f_unexpected_but_if(CTX->m_err) goto LBL_FAIL;

					FNCTX->m_state0		|= FNCTX_NEW.m_state0;

					ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
								CTX->m_num_cursor
								, CTX->m_cursors.m_p
								, CTX)) goto LBL_FAIL;

					unless(FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
						ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
									&CTX->m_section.m_fnimpl
									, CTX->m_count.m_fnimpl
									, SpvOpBranch
									, IFPART.m_b_default
									)) goto LBL_FAIL;
					}
				}

				if(IFPART.m_b_false ^ IFPART.m_b_default /* != */) {
					aclutil_fnctx		FNCTX_NEW = *FNCTX;
					FNCTX_NEW.m_state0 = 0;

					ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
								&CTX->m_section.m_fnimpl
								, CTX->m_count.m_fnimpl
								, SpvOpLabel
								, IFPART.m_b_false
								)) goto LBL_FAIL;

					clang_visitChildren(
							IFPART.m_parts[2]
							, aclemit_entp_body
							, &FNCTX_NEW
							); ae2f_unexpected_but_if(CTX->m_err) goto LBL_FAIL;

					FNCTX->m_state0		|= FNCTX_NEW.m_state0;

					ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
								CTX->m_num_cursor
								, CTX->m_cursors.m_p
								, CTX)) goto LBL_FAIL;

					unless(FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
						ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
									&CTX->m_section.m_fnimpl
									, CTX->m_count.m_fnimpl
									, SpvOpBranch
									, IFPART.m_b_default
									)) goto LBL_FAIL;
					}
				}

				ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
							&CTX->m_section.m_fnimpl
							, CTX->m_count.m_fnimpl
							, SpvOpLabel
							, IFPART.m_b_default
							)) goto LBL_FAIL;
			} goto LBL_DONE;

		case CXCursor_UnaryOperator:
		case CXCursor_CallExpr:
		case CXCursor_ArraySubscriptExpr:
		case CXCursor_ConditionalOperator:
		case CXCursor_CStyleCastExpr:
		case CXCursor_DeclRefExpr:
		case CXCursor_UnaryExpr:
		case CXCursor_DoStmt:

		case CXCursor_ForStmt:
		case CXCursor_WhileStmt:
		case CXCursor_BreakStmt:
		case CXCursor_ContinueStmt:

		case CXCursor_SwitchStmt:
		case CXCursor_CaseStmt:
		case CXCursor_AsmStmt: case CXCursor_MSAsmStmt:


		case CXCursor_StmtExpr:
		default:
			break;
	}


	warn_prefix(i);
	warn_call(fprintf, (stderr, "Met unprocessed spell(%s) in kind(%s) [NOIMPL, BODY]\n"
				, (const char*)SPELL.data
				, (const char*)KINDSPELL.data));

	CTX->m_err = ACLSPV_COMPILE_OK;
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Recurse;

	ae2f_unreachable();
LBL_FAIL:
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Break;

	ae2f_unreachable();
LBL_DONE:
	CTX->m_err = ACLSPV_COMPILE_OK;
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Continue;

	ae2f_unreachable();
LBL_RECURSE:
	CTX->m_err = ACLSPV_COMPILE_OK;
	clang_disposeString(SPELL);
	clang_disposeString(KINDSPELL);
	return CXChildVisit_Recurse;

#undef	CTX
#undef	jmpfail
#undef	___mkname_on_dbg
}

