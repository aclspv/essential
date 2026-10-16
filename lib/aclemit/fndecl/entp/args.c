#include "./args.h"

#include <ae2f/bll.h>
#include <ae2f/pat/dbg.h>

#include <util/scale.h>
#include <util/wrdemit.h>
#include <util/constant.h>
#include <util/emitx.h>
#include <util/iddef.h>

#include <aclty.h>

#include <aclval/bind.h>

#include <aclspv.h>
#include <aclmem.h>

#include <aclutil/extension.h>

#include <aclctx/err.h>
#include <aclctx/id.h>

#include <aclcur/common.h>
#include <aclcur/reg/var.h>
#include <aclcur/reg/view.h>

#include <aclval.h>

#include <aclattr/storage_class.h>
#include <aclattr/location.h>
#include <aclattr/layout.h>

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

static enum CXChildVisitResult	s_attr_bind_idx(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict h_data) {
	CXString TEXT;
	enum CXChildVisitResult RES = CXChildVisit_Break;
	const char* NEEDLE;
	unsigned SET;

	unless(h_cur.kind == CXCursor_AnnotateAttr) return CXChildVisit_Recurse;
	TEXT = clang_getCursorSpelling(h_cur);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_bind_idx")) {
		RES = CXChildVisit_Continue;
		goto LBL_FINI;
	}

	sscanf(NEEDLE, "aclspv_set ( %u )", &SET);

	*((aclspv_wrd_t* ae2f_restrict)h_data) = (aclspv_wrd_t)SET;

LBL_FINI:
	clang_disposeString(TEXT);
	return RES;
}

/** the index chain an argument register's pointer value needs instead of a
 * plain pointer copy.
 *
 * a storage-buffer (or workgroup) pointer argument is materialised as an
 * OpVariable whose pointee is a single-member wrapper struct (the Vulkan
 * block idiom) while the register's logical type is `ptr<SC,L>`. the
 * register's native view (m_id_native) carries member 0's type; the pointer
 * value is that member's address:
 *
 *   - `struct T*` -> `struct { struct T }`: one index, member 0.
 *   - `T*`        -> `struct { T[] }`:      two indices, member 0 (the
 *                                           runtime array), then element 0.
 *
 * the register's value is therefore NOT the variable id: it is the address
 * the chain lands on, taking the same shape as the subscript carve's chain.
 *
 * only the pointer argument idiom is descended; an array-typed logical
 * argument (whose value is not an address) keeps the historical copy.
 *
 * @param c_mbr0 member 0's type id (the native view's m_ty), 0 when the
 * register has no wrapper.
 * @return 1 and fills the shared i32 0 constant id, plus the index count
 * (1 or 2), when the descent applies.
 * */
static aclspv_bool_t	s_argv_load_chain(
		const aclspv_id_t	c_mbr0,
		const aclspv_id_t	c_logical_ty,
		h_aclspv_ctx_t		h_ctx,
		aclspv_id_t* ae2f_restrict	rdwr_idx0,
		aclspv_wrd_t* ae2f_restrict	rdwr_num_idx
		) {
	const aclty_t* ae2f_restrict P_LOGICAL;
	const aclty_t* ae2f_restrict P_M0;
	aclspv_id_t	id_log;

	if(!c_mbr0)
		return 0;

	P_LOGICAL	= aclty_find(c_logical_ty, h_ctx);

	/** only the pointer argument idiom: the register's logical type is
	 * `ptr<SC,L>`; its value denotes the address of L. */
	if(!P_LOGICAL || P_LOGICAL->m_class != ACLTY_PTR)
		return 0;

	id_log		= P_LOGICAL->m_info.m_ptr.m_type;

	P_M0		= aclty_find(c_mbr0, h_ctx);

	if(!P_M0)
		return 0;

	if(P_M0->m_class == ACLTY_ARR) {
		/** `T* -> struct { T[] }`: member 0, then element 0. */
		if(P_M0->m_info.m_arr.m_type != id_log)
			return 0;

		*rdwr_num_idx	= 2;
	} else if(c_mbr0 == id_log) {
		/** `struct T* -> struct { struct T }`: member 0 only. */
		*rdwr_num_idx	= 1;
	} else {
		return 0;
	}

	ae2f_ifezerr(*rdwr_idx0 = aclutil_mk_constant_val_id(0, h_ctx))
		return 0;

	return 1;
}

/** POINTER-flavored argument load: an entry-point arg register IS its
 * interface variable (m_id == the OpVariable result id), so "loading"
 * it copies the pointer — the result is a non-dominating alias of the
 * same storage. a pointer copy is not a memory read, so the explicit
 * (volatile) variant is the same op, matching the logical-addressing
 * model.
 *
 * when the variable's storage type is a wrapped single-member struct but
 * the register's logical type is the plain pointee pointer, the copy's
 * operand type would not match its result type — the address of the
 * wrapper's member 0 (element 0 for the runtime array idiom) is
 * materialised instead (see s_argv_load_chain). */
static aclspv_compile_t	s_argv_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	aclcur_t* ae2f_restrict p;
	aclcur_t* ae2f_restrict p_reg;

	ae2f_ifezerr(p = aclcur_get_node(c_id, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_LOAD)
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifezerr(p_reg = aclcur_get_node(p->m_d.m_load.m_reg, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
		return ACLSPV_COMPILE_UNHANDLED;

	{
		aclspv_id_t IDX0;
		aclspv_wrd_t NUM_IDX;
		aclcur_t* ae2f_restrict p_native = p_reg->m_d.m_reg.m_id_native
				? aclcur_get_node(p_reg->m_d.m_reg.m_id_native, h_ctx)
				: 0;

		if(s_argv_load_chain(
					p_native ? p_native->m_ty : 0
					, p_reg->m_ty
					, h_ctx
					, &IDX0
					, &NUM_IDX
					)) {
			/** two indices (member 0 then element 0) for the runtime
			 * array wrapper; one (member 0) for the struct wrapper. */
			if(NUM_IDX >= 2) {
				ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_6(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpAccessChain
							, p_reg->m_ty
							, c_id
							, p_reg->m_id
							, IDX0
							, IDX0
							)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
			} else {
				ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_5(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpAccessChain
							, p_reg->m_ty
							, c_id
							, p_reg->m_id
							, IDX0
							)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
			}
		} else {
			ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_4(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpCopyObject
						, p_reg->m_ty
						, c_id
						, p_reg->m_id
						)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
		}
	}

	return ACLSPV_COMPILE_OK;
}

/** OpAccessChain index-chain depth cap (fixed-size accumulator),
 * mirroring lib/aclcur/reg/view.c's ACLCUR_REGVIEW_CHAIN_MAX. */
#define	ACLCUR_ARG_PCV_CHAIN_MAX		__ACLSPV_OPACC_IDX_MAX

/** pointer producer for push-constant leaf registers: appends one
 * OpAccessChain per ALIAS2 level (root-most first) directly into the
 * function body as raw words — the indices are module-level constants
 * and the base is the shared OpVariable id, so no op-node minting at
 * emit time (which would land at the buffer tail / be elided). */
static aclspv_compile_t	s_pcv_chain(
		aclspv_id_t c_reg
		, aclspv_id_t* ae2f_restrict wr_ptr
		, h_aclspv_ctx_t h_ctx
		) {
	aclspv_id_t idx_s[ACLCUR_ARG_PCV_CHAIN_MAX];
	aclspv_id_t tys[ACLCUR_ARG_PCV_CHAIN_MAX];
	aclspv_wrd_t nidx;
	aclspv_wrd_t bound;
	aclspv_id_t id_cur;
	h_aclcur_t p;
	h_aclcur_t p_self;
	aclspv_id_t id_base;
	aclspv_wrd_t i;

	/** the leaf register whose address is requested. */
	ae2f_ifezerr(p_self = aclcur_get_node(c_reg, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p_self->m_class != ACLCUR_REG)
		return ACLSPV_COMPILE_UNHANDLED;

	/** ancestor walk: view-first, collecting each m_id_idx together
	 * with the type that step lands on; the root is reached when the
	 * ALIAS2 flag is absent. iterative and bounded, exactly as
	 * lib/aclcur/reg/view.c's addr walk. */
	nidx	= 0;
	bound	= h_ctx ? h_ctx->m_num_cursor : 0;
	id_cur	= c_reg;
	p	= 0;
	while(bound-- && nidx < ACLCUR_ARG_PCV_CHAIN_MAX) {
		ae2f_ifezerr(p = aclcur_get_node(id_cur, h_ctx))
			return ACLSPV_COMPILE_UNHANDLED;
		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
			return ACLSPV_COMPILE_UNHANDLED;

		unless(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
			break;

		idx_s[nidx]	= p->m_d.m_reg.m_id_idx;
		tys[nidx]	= p->m_ty;
		++nidx;
		id_cur		= p->m_d.m_reg.m_d.m_ref.m_id;
	}

	/** the walk must have stopped on a root: exhausting the bound
	 * (cyclic parent chain) or the accumulator is a failure. */
	ae2f_ifezerr(p)
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		return ACLSPV_COMPILE_UNHANDLED;

	/** first chain step's base: the root register's id, which IS the
	 * shared OpVariable result id (m_id fixup in the binding switch). */
	id_base	= p->m_id;

	for(i = 0; i < nidx; ++i) {
		const aclspv_wrd_t J = nidx - 1 - i;
		const aclspv_id_t TY_ELEM = aclty_mk_ptr(
				tys[J]
				, (enum SpvStorageClass_)p_self->m_d.m_reg.m_storage_class
				, h_ctx
				);
		const aclspv_id_t id_nxt = aclctx_newid(h_ctx);

		ae2f_ifezerr(TY_ELEM)
			return ACLSPV_COMPILE_ALLOC_FAILED_ID;
		ae2f_ifezerr(id_nxt)
			return ACLSPV_COMPILE_ALLOC_FAILED_ID;

		/** OpAccessChain word layout: [op|5] [result type] [result id]
		 * [base] [index], one index per level. the result type is the
		 * pointer to the level's own type under the leaf's storage
		 * class, exactly as reg/view.c derives it. */
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_5(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpAccessChain
					, TY_ELEM
					, id_nxt
					, id_base
					, idx_s[J]
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

		id_base	= id_nxt;
	}

	*wr_ptr	= id_base;
	return ACLSPV_COMPILE_OK;
}

/** push-constant leaf load: parameterized form of the stock variable
 * load, with the chain producer supplying the memory operand. */
static aclspv_compile_t	s_pcv_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_pcv_chain, 0);
}

/** explicit (volatile) variant of the push-constant leaf load. */
static aclspv_compile_t	s_pcv_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_loadof(c_id, h_ctx, s_pcv_chain, 1);
}

/** push-constant leaf store: the non-explicit gating (pure SSA
 * definition, no memory write) lives inside the of-variant. */
static aclspv_compile_t	s_pcv_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_pcv_chain, 0);
}

/** explicit push-constant leaf store. */
static aclspv_compile_t	s_pcv_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	return aclcur_regvar_emit_storeof(c_id, h_ctx, s_pcv_chain, 1);
}

enum CXChildVisitResult aclemit_fndecl_entp_arg(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData ae2f_restrict h_data
		) {
	CXType		PARAM_TY;
	struct aclemit_fndecl_entp_arg_data* ae2f_restrict DAT = h_data;

#define CTX		DAT->m_ctx 

	/** global: storage class */
	aclspv_wrd_t	storage_class;

	/** global: variable id */
	aclspv_id_t	var_id;
	/** global: variable as native */
	aclspv_id_t	var_native_type_id = 0;
	/** global: from cursor */
	aclspv_id_t	var_type_id;
	aclspv_id_t	var_elm_type_id;

	/** for bindables only */
	aclspv_wrd_t	bind_set = 0xFFFFFFFF;
	aclspv_wrd_t	bind_idx = 0xFFFFFFFF;

	/** push constant: to access the right argument */
	aclspv_wrd_t	pc_idx_func = 0;
	aclspv_wrd_t	pc_idx_arg = 0;

	ae2f_ifezerr(CTX)		return CXChildVisit_Break;
	ae2f_ifnezerr(CTX->m_err) 	return CXChildVisit_Break;

	ae2f_ifnezerr_strict(h_cur.kind != CXCursor_ParmDecl) {
		return CXChildVisit_Continue;
	}

	PARAM_TY	= clang_getCursorType(h_cur);

	storage_class		= SpvStorageClassPushConstant;

	/** Attriburte: storage class */
	clang_visitChildren(
			h_cur
			, s_attr_storage_class
			, &storage_class
			);

#define	storage_class	((const aclspv_wrd_t)storage_class)

	/** 
	 * var_type_id 
	 * var_elm_type_id
	 * */
	switch(storage_class) {
		default:
		case SpvStorageClassMax:
			_aclctx_err(CTX, ACLSPV_COMPILE_STORAGE_CLASS_UNDEFINED);
			return CXChildVisit_Break;

			/** 
			 * those are expected to be 
			 * - a pointer to a struct
			 * - a pointer to anything
			 * - an array to something
			 *
			 * on struct: this should be understood as a layout.
			 * on anything: this should be understood as a runtime array
			 **/
		case SpvStorageClassStorageBuffer:

			/*** Extension Default **/
			ae2f_ifezerr(aclutil_add_extension_by_flag(CTX, "SPV_KHR_storage_buffer_storage_class")) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				return CXChildVisit_Break;
			}

			ae2f_ifezerr(var_type_id = aclty_mk_by_cursor_dummy(
						h_cur
						, PARAM_TY
						, CTX
						)) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				return CXChildVisit_Break;
			}

			{
				aclty_t* ae2f_restrict p_ty = aclty_find(var_type_id, CTX);
				ae2f_ifezerr(p_ty) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				switch(p_ty->m_class) {
					/** T[] -> struct { T[] } */
					case ACLTY_ARR:
						var_elm_type_id = var_type_id;
						ae2f_ifezerr(var_elm_type_id = aclty_mk_struct_single_blk(
									var_elm_type_id
									, ACLTY_STRUCT_FIELD0_BLOCK
									, 0
									, CTX
									)) {
							_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
							return CXChildVisit_Break;
						}

						break;

						/** 
						 * struct T* -> struct { struct T } 
						 * T* -> struct { T[] }
						 * */
					case ACLTY_PTR:
						{
							const aclspv_id_t	id_elm = p_ty->m_info.m_ptr.m_type;
							aclty_t* ae2f_restrict p_elm = aclty_find(id_elm, CTX);

							ae2f_ifnezerr_strict(!p_elm && id_elm >= ACLSPV_ID_END) {
								_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
								return CXChildVisit_Break;
							}

							if(p_elm && p_elm->m_class == ACLTY_STRUCT) {
								ae2f_ifezerr(var_elm_type_id = aclty_mk_struct_single_blk(
											id_elm
											, ACLTY_STRUCT_FIELD0_BLOCK
											, 0
											, CTX
											)) {
									_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
									return CXChildVisit_Break;
								}
							} else {
								aclconst_specsz a;
								a.m_id = 0;
								a.m_lit.m_max = 0;

								ae2f_ifezerr(var_elm_type_id = aclty_mk_arr3(
											id_elm
											, a
											, 4
											, CTX
											)) {
									_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
									return CXChildVisit_Break;
								}

								ae2f_ifezerr(var_elm_type_id = aclty_mk_struct_single_blk(
											var_elm_type_id
											, ACLTY_STRUCT_FIELD0_BLOCK
											, 0
											, CTX
											)) {
									_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
									return CXChildVisit_Break;
								}
							}
						}
						break;

					case ACLTY_STRUCT:
					case ACLTY_DCR:
					case ACLTY_FN:
					case ACLTY_FN_ARG:
					case ACLTY_VEC:
					case ACLTY_RESERVED:
					case ACLTY_STRUCT_MBR:
					default:
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
				}

			}

			break;

			/** this is an only type which type should NOT be a pointer */
		case SpvStorageClassPushConstant:
			ae2f_ifezerr(var_type_id = aclty_mk_by_cursor_dummy(
						h_cur
						, PARAM_TY
						, CTX
						)) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				return CXChildVisit_Break;
			}

			var_elm_type_id = var_type_id;

			ae2f_ifezerr(aclty_getsize(var_elm_type_id, DAT->m_ctx).m_id || aclty_getsize(var_elm_type_id, DAT->m_ctx).m_lit.m_max) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				err_prefix(i);
				err_call(fprintf, (stderr, "is abstract. and is push constant. is shit. %lu\n", (unsigned long)var_type_id));
				return CXChildVisit_Break;
			}

			break;

		case SpvStorageClassInput:
		case SpvStorageClassOutput:
			ae2f_ifezerr(var_type_id = aclty_mk_by_cursor_dummy(
						h_cur
						, PARAM_TY
						, CTX
						)) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				return CXChildVisit_Break;
			}

			{
				aclty_t* ae2f_restrict p_ty = aclty_find(var_type_id, CTX);
				ae2f_ifezerr(p_ty) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifnezerr_strict(p_ty->m_class != ACLTY_PTR) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				{
					aclty_t* ae2f_restrict p_elm = aclty_find(p_ty->m_info.m_ptr.m_type, CTX);

					ae2f_ifnezerr_strict(!p_elm && p_ty->m_info.m_ptr.m_type >= ACLSPV_ID_END) {
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
					}

					var_elm_type_id = p_ty->m_info.m_ptr.m_type;
				}
			}

			ae2f_ifezerr(aclty_getsize(var_elm_type_id, DAT->m_ctx).m_id || aclty_getsize(var_elm_type_id, DAT->m_ctx).m_lit.m_max) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				err_prefix(i);
				err_call(fprintf, (
							stderr
							, "is abstract. and is input/output. is shit. %lu\n"
							, (unsigned long)var_type_id)
						);
				return CXChildVisit_Break;
			}

			break;

			/**
			 * for uniform constant
			 * 
			 * std140 is natively supported.
			 * std430 could be supported under extension & core feature of vulkan
			 * */
		case SpvStorageClassUniform:
		case SpvStorageClassUniformConstant:
			switch(aclattr_get_layout(h_cur, PARAM_TY, CTX)) {
				case ACLSPV_LAYOUT_STD430:
					/** standard layout under vulkan 1.2 */
					if(CTX->m_usrcfg.m_ver_vk < aclspv_ver_vk2(1, 2)) ae2f_ifezerr(aclutil_add_extension_by_flag(
								CTX
								, "VK_KHR_uniform_buffer_standard_layout")) {
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
					}

					break;

				case ACLSPV_LAYOUT_STD140:
					break;	/** nothing wrong here - that's how uniform works */

				default:
					/** idk what this is */
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
			}

			ae2f_fallthrough;

			/** 
			 * those are expected to be 
			 * - a pointer to a struct
			 * - a pointer to anything
			 * - an array of something
			 *
			 * and pointee should have constant size
			 *
			 * on struct: this should be understood as a layout.
			 * on anything: this should be understood as a thing inside a struct
			 **/
		case SpvStorageClassCrossWorkgroup:
		case SpvStorageClassWorkgroup:
			ae2f_ifezerr(var_type_id = aclty_mk_by_cursor_dummy(
						h_cur
						, PARAM_TY
						, CTX
						)) {
				_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
				return CXChildVisit_Break;
			}

			{
				aclty_t* ae2f_restrict p_ty = aclty_find(var_type_id, CTX);

				/** 
				 * any fixed-size type is accepted here:
				 *   struct | int | float | vector | constant-size array.
				 *
				 * a builtin scalar | vector has no type node (p_ty == 0) and is
				 * already fixed-size, so it is taken as-is.
				 *
				 * a runtime-sized array (`T[]` without specid) and anything else
				 * abstract is rejected by the size guard below.
				 **/
				var_elm_type_id = var_type_id;

				ae2f_ifnezerr(p_ty) switch((aclspv_wrd_t)p_ty->m_class) {
					/** T[] -> struct { T[] } */
					case ACLTY_ARR:
						ae2f_ifezerr(var_elm_type_id = aclty_mk_struct_single_blk(
									var_elm_type_id
									, ACLTY_STRUCT_FIELD0_BLOCK
									, 0
									, CTX
									)) {
							_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
							return CXChildVisit_Break;
						}

						break;

						/** 
						 * T* -> struct { T }
						 * */
					case ACLTY_PTR:
						{
							aclty_t* ae2f_restrict p_elm = aclty_find(p_ty->m_info.m_ptr.m_type, CTX);
							ae2f_ifnezerr_strict(!p_elm && p_ty->m_info.m_ptr.m_type >= ACLSPV_ID_END) {
								_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
								return CXChildVisit_Break;
							}

							var_elm_type_id = p_ty->m_info.m_ptr.m_type;
							ae2f_ifezerr(var_elm_type_id = aclty_mk_struct_single_blk(
										var_elm_type_id
										, ACLTY_STRUCT_FIELD0_BLOCK
										, 0
										, CTX
										)) {
								_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
								return CXChildVisit_Break;
							}
						}
						break;

					default:
						/** fixed-size value (struct, vector, ...): used as-is. */
						break;
				}
			}

			ae2f_ifezerr(aclty_getsize(var_elm_type_id, DAT->m_ctx).m_id || aclty_getsize(var_elm_type_id, DAT->m_ctx).m_lit.m_max) {
				_aclctx_err(
						DAT->m_ctx
						, ACLSPV_COMPILE_UNHANDLED_ARGS
						);
				err_prefix(i);
				err_call(fprintf, (stderr, "is abstract. and is workgroup | uniform. is shit. %lu\n", (unsigned long)var_elm_type_id));
				return CXChildVisit_Break;
			}

			break;
	}

	/** guard to make it constant */

#define	var_type_id	((const aclspv_id_t)var_type_id)
#define	var_elm_type_id	((const aclspv_id_t)var_elm_type_id)

	/**
	 * variable emission
	 *
	 * - var_id
	 * - bindable_idx
	 * - bindable_set
	 * - pc_idx_func
	 * - pc_idx_arg
	 * */
	switch(storage_class) {
		default:
		case SpvStorageClassMax:
			ae2fpat_assertassume_strict(0 && "Storage Class Undefined && dead code");
			ae2f_unreachable();


		case SpvStorageClassPushConstant:
			{
				aclspv_id_t ty_old = aclty_cfg_struct2(CTX, DAT->m_id_push_constant_struct_ty);
				aclty_t* ae2f_restrict p_old;

				ae2f_ifezerr(p_old = aclty_find(ty_old, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(aclty_add_struct_mbr_std430(p_old, var_elm_type_id, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(p_old = aclty_build_struct(p_old->m_type_id, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				dbg_prefix(a);
				dbg_call(fprintf, (stderr, "number member of push constant (arg): %u\n"
							, (unsigned)p_old->m_info.m_struct.m_num_mbr));

				DAT->m_id_push_constant_struct_ty = p_old->m_type_id;

				pc_idx_func	= CTX->m_num_push_constant_func;
				pc_idx_arg	= DAT->m_idx_push_constant_prm++;

				if (CTX->m_push_size_ty_pad) {
					/** the shared OpVariable was emitted once into
					 * m_section.m_vars by the first push-constant
					 * argument; its result id sits at the word right
					 * after the type word (m_push_size_ty_pad is the
					 * type-word index, patched at finalization). */
					var_id = DAT->m_id_push_constant = aclutil_get_wrd_of_vec(
							&CTX->m_section.m_vars
							)[CTX->m_push_size_ty_pad + 1];
				} else {
					CTX->m_push_size_ty_pad = DAT->m_ctx->m_count.m_vars + 1;
					ae2f_ifezerr(DAT->m_ctx->m_count.m_vars
							= aclutil_emitx_4(
								&DAT->m_ctx->m_section.m_vars
								, DAT->m_ctx->m_count.m_vars
								, SpvOpVariable
								, 0
								, var_id = DAT->m_id_push_constant = aclctx_newid(CTX)
								, SpvStorageClassPushConstant
								)) {
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
					}
				}
			}

			break;

			/** bindables: much be identical under same descriptor set */
		case SpvStorageClassUniformConstant:
		case SpvStorageClassUniform:
		case SpvStorageClassStorageBuffer:
			{
				var_native_type_id = aclty_mk_ptr(
						var_elm_type_id
						, (SpvStorageClass)storage_class
						, CTX
						);

				ae2f_ifezerr(var_native_type_id) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				clang_visitChildren(h_cur
						, s_attr_set
						, &bind_set
						);

				clang_visitChildren(h_cur
						, s_attr_bind_idx
						, &bind_idx
						);

				ae2f_ifnezerr_strict(bind_set != 0xFFFFFFFF && bind_idx == 0xFFFFFFFF) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				if(bind_set == 0xFFFFFFFF)
					bind_set = DAT->m_entp.m_descriptor_set;

				/** only when bindable set is not defined */
				if(bind_idx == 0xFFFFFFFF)
					bind_idx = aclval_inc_bind_last_index(bind_set, CTX);

				ae2f_ifnezerr_strict( 0xFFFFFFFF == bind_idx) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifnezerr_strict( 0xFFFFFFFF == bind_set) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifnezerr_strict( storage_class != aclval_cfg_bind_storage_class(
							bind_set
							, bind_idx
							, storage_class
							, CTX) ) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(CTX->m_count.m_decorate = aclutil_emitx_4(
							/** Decorations::Descriptorsets */
							&CTX->m_section.m_decorate
							, CTX->m_count.m_decorate
							, SpvOpDecorate
							, var_id = aclctx_newid(CTX)
							, SpvDecorationDescriptorSet
							, bind_set
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(CTX->m_count.m_decorate = aclutil_emitx_4(
							/** Decorations::Binding */
							&CTX->m_section.m_decorate
							, CTX->m_count.m_decorate
							, SpvOpDecorate
							, var_id
							, SpvDecorationBinding
							, bind_idx
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(CTX->m_count.m_vars = aclutil_emitx_4(
							&CTX->m_section.m_vars
							, CTX->m_count.m_vars
							, SpvOpVariable
							, var_native_type_id
							, var_id
							, storage_class
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}
			}

			break;
		case SpvStorageClassWorkgroup:
			{
				ae2f_ifezerr(var_native_type_id = aclty_mk_ptr(
							var_elm_type_id
							, SpvStorageClassWorkgroup
							, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(CTX->m_count.m_vars = util_emitx_variable(
							&CTX->m_section.m_vars
							, CTX->m_count.m_vars
							, var_native_type_id
							, var_id = aclctx_newid(CTX)
							, SpvStorageClassWorkgroup
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}
			}

			break;

		case SpvStorageClassInput:
		case SpvStorageClassOutput:
			{
				aclspv_wrd_t location = 0xFFFFFFFF;

				var_native_type_id = aclty_mk_ptr(
						var_elm_type_id
						, (SpvStorageClass)storage_class
						, CTX
						);

				clang_visitChildren(
						h_cur
						, s_attr_location
						, &location
						);

				if(location == 0xFFFFFFFF)
					location = 0;

				ae2f_ifezerr(CTX->m_count.m_decorate = aclutil_emitx_4(
							&CTX->m_section.m_decorate
							, CTX->m_count.m_decorate
							, SpvOpDecorate
							, var_id = aclctx_newid(CTX)
							, SpvDecorationLocation
							, location
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(CTX->m_count.m_vars = util_emitx_variable(
							&CTX->m_section.m_vars
							, CTX->m_count.m_vars
							, var_native_type_id
							, var_id
							, storage_class
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}
			}

			break;
	}

#define	var_id			((const aclspv_id_t)var_id)
#define	var_native_type_id	((const aclspv_id_t)var_native_type_id)

#define	bindable_set		((const aclspv_wrd_t)bindable_set)
#define	bindable_idx		((const aclspv_wrd_t)bindable_idx)
#define	pc_idx_func		((const aclspv_wrd_t)pc_idx_func)
#define	pc_idx_arg		((const aclspv_wrd_t)pc_idx_arg)

	/** 
	 * adding to entry point as interface
	 * */
	switch(storage_class) {
		case SpvStorageClassMax:
		default:
			ae2fpat_assertassume_strict(0 && "Storage Class Undefined && dead code");
			ae2f_unreachable();


		case SpvStorageClassPushConstant:
			if(DAT->m_id_push_constant_struct_ty)
				break;

			ae2f_fallthrough;
		case SpvStorageClassWorkgroup:
		case SpvStorageClassStorageBuffer:
		case SpvStorageClassUniform:
		case SpvStorageClassUniformConstant:
			/** the OpEntryPoint interface may only list Input/Output
			 * variables (SPIR-V 1.3 and earlier). Uniform/Workgroup/
			 * PushConstant storage never joins the interface list. */
			if(DAT->m_ctx->m_usrcfg.m_ver_spv < aclspv_ver_spv(1, 4))
				break;

			ae2f_fallthrough;
		case SpvStorageClassInput:
		case SpvStorageClassOutput:
			ae2f_ifnezerr_strict(s_aclmem_cgrow2(
						&CTX->m_section.m_entp
						, CTX->m_count.m_entp + 1
						, sizeof(aclspv_wrd_t)
						, ACLSPV_COMPILE_ALLOC_FAILED_ENTP_ARGS))
				return CXChildVisit_Break;

			((aclspv_wrd_t* ae2f_restrict)CTX->m_section.m_entp.m_p)[CTX->m_count.m_entp++]
				= var_id;

			((aclspv_wrd_t* ae2f_restrict)CTX->m_section.m_entp.m_p)[DAT->m_idx_entp_buf_count]
				+= 0x10000;
	}

	/** 
	 * register binding: the arg cursor resolves to its interface
	 * variable. m_id == var_id makes the register id the OpVariable
	 * result id, so expressions can load/store the entry-point argument.
	 * */
	switch(storage_class) {
		case SpvStorageClassMax:
		default:
			ae2fpat_assertassume_strict(0 && "Storage Class Undefined && dead code");
			ae2f_unreachable();

		case SpvStorageClassPushConstant:
			{
				aclspv_id_t cst_func, cst_arg, reg_leaf;
				aclcur_t* ae2f_restrict p_view;

				/** member ordinals as i32 OpConstants. */
				ae2f_ifezerr(cst_func = aclutil_mk_constant_val_id(
							pc_idx_func
							, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}
				ae2f_ifezerr(cst_arg = aclutil_mk_constant_val_id(
							pc_idx_arg
							, CTX
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				/** packing-level view R1 over the shared variable,
				 * once per function. */
				unless(DAT->m_id_push_constant_reg_view) {
					aclspv_id_t reg_root;
					aclcur_t* ae2f_restrict p_root;

					/** per-function root register (R0): the cursor
					 * arena resets per function, and the shared
					 * push-constant variable belongs to the module,
					 * so recreate the root each function; its m_id IS
					 * the shared OpVariable result id so R1/leaf
					 * chains resolve it. */
					ae2f_ifezerr(reg_root = aclcur_cfg_reg(
								clang_getNullCursor()
								, 0
								, SpvStorageClassPushConstant
								, 0
								, CTX
								)) {
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
					}
					ae2f_ifezerr(p_root = aclcur_get_node(reg_root, CTX)) {
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
					}
					p_root->m_id = var_id;

					ae2f_ifezerr(DAT->m_id_push_constant_reg_view = aclcur_reg_view(
								clang_getNullCursor()
								, var_id
								, DAT->m_id_push_constant_struct_ty
								, cst_func
								, SpvStorageClassPushConstant
								, ACLCUR_REGVIEW_STEP_MEMBER
								, CTX
								)) {
						_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
						return CXChildVisit_Break;
					}
				}

				/** the argument's member view: bound to h_cur so
				 * expressions find it. */
				ae2f_ifezerr(reg_leaf = aclcur_reg_view(
							h_cur
							, DAT->m_id_push_constant_reg_view
							, var_elm_type_id
							, cst_arg
							, SpvStorageClassPushConstant
							, ACLCUR_REGVIEW_STEP_MEMBER
							, CTX
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				ae2f_ifezerr(p_view = aclcur_get_node(reg_leaf, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}
				p_view->m_d.m_reg.m_in.m_emit_load		= s_pcv_emit_load;
				p_view->m_d.m_reg.m_in.m_emit_load_explicit	= s_pcv_emit_load_explicit;
				p_view->m_d.m_reg.m_in.m_emit_store		= s_pcv_emit_store;
				p_view->m_d.m_reg.m_in.m_emit_store_explicit	= s_pcv_emit_store_explicit;
			}
			break;

		case SpvStorageClassStorageBuffer:
		case SpvStorageClassUniform:
		case SpvStorageClassUniformConstant:
		case SpvStorageClassWorkgroup:
		case SpvStorageClassInput:
		case SpvStorageClassOutput:
			{
				aclcur_reg_t REGIN = aclcur_regvar_in();
				aclspv_id_t reg;
				aclcur_t* ae2f_restrict p_reg;

				REGIN.m_in.m_emit_reg = 0;
				REGIN.m_in.m_emit_load = s_argv_emit_load;
				REGIN.m_in.m_emit_load_explicit = s_argv_emit_load;

				ae2f_ifezerr(reg = aclcur_cfg_reg(
							h_cur
							, var_type_id
							, storage_class
							, &REGIN
							, CTX
							)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}

				/** the register id must BE the OpVariable result id. */
				ae2f_ifezerr(p_reg = aclcur_get_node(reg, CTX)) {
					_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
					return CXChildVisit_Break;
				}
				p_reg->m_id = var_id;
				/** the OpVariable's storage type, when it differs from the
				 * register's logical var_type_id (wrapped-argument idiom):
				 * the pointer value is member 0's address, so keep the
				 * MEMBER view of member 0 over the variable — its chain is
				 * that address, its m_ty member 0's type. 0 for the
				 * Input/Output branch (no wrapper) and the push-constant
				 * branch above. */
				p_reg->m_d.m_reg.m_id_native = 0;

				if(var_native_type_id && var_native_type_id != var_type_id) {
					aclspv_id_t cst0, id_nat;
					aclty_t* ae2f_restrict p_elm
						= aclty_find(var_elm_type_id, CTX);
					const aclty_t* ae2f_restrict p_mbr0
						= p_elm
						? aclty_get_struct_mbr(p_elm, 0, CTX)
						: 0;

					if(p_mbr0) {
						ae2f_ifezerr(cst0 = aclutil_mk_constant_val_id(0, CTX)) {
							_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
							return CXChildVisit_Break;
						}

						/** ELEMENT step (not MEMBER) so this view is
						 * canonical with the subscript carve's member-0
						 * intermediate view: the same (parent, step, idx,
						 * ty) tuple, hence one node (see
						 * aclcur_reg_view's dedup). */
						ae2f_ifezerr(id_nat = aclcur_reg_view(
									clang_getNullCursor()
									, var_id
									, p_mbr0->m_info.m_struct_mbr.m_id_mbr
									, cst0
									, storage_class
									, ACLCUR_REGVIEW_STEP_ELEMENT
									, CTX
									)) {
							_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
							return CXChildVisit_Break;
						}

						/** aclcur_reg_view allocates; and m_id is now
						 * var_id, so re-fetch by that. */
						ae2f_ifezerr(p_reg = aclcur_get_node(var_id, CTX)) {
							_aclctx_err(DAT->m_ctx, ACLSPV_COMPILE_UNHANDLED_ARGS);
							return CXChildVisit_Break;
						}
						p_reg->m_d.m_reg.m_id_native = id_nat;
					}
				}
			}
			break;
	}

	return CXChildVisit_Continue;
}
