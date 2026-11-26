#include "./norm.h"

#include <aclsym/fn.h>

#include <aclattr/fnctrl.h>

#include <aclemit/fndecl/body.h>
#include <aclemit/fndecl/sym.h>
#include <aclemit/root.h>
#include <aclemit/dtor.h>

#include <aclcur/common.h>
#include <aclcur/reg.h>
#include <aclcur/reg/var.h>
#include <aclcur/store.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <aclutil/emitx.h>
#include <aclutil/capability.h>
#include <aclutil/extension.h>
#include <aclutil/fn.h>

#include <ae2f/pat/dbg.h>

/** a normal function body: OpFunction, its parameters, then the body.
 *
 * one function per invocation. the cursor arena was bootstrapped by
 * aclemit_root with a void return type; the root label's m_ty is the
 * function return type, so it is patched before the body is carved.
 * */
enum CXChildVisitResult aclemit_fndecl_norm(
		CXCursor h_cur,
		CXCursor h_parent,
		CXClientData h_ctx
		) {
	h_aclspv_ctx_t			ctx = h_ctx;
	aclsym_fn_t* ae2f_restrict	p_sym;
	aclty_t* ae2f_restrict		p_root_ty;
	aclcur_t* ae2f_restrict		p_root;
	aclspv_id_t			id_fn_ty;
	aclspv_id_t			id_ret;
	aclspv_wrd_t			c_fnctrl;
	aclspv_sz_t			c_fn_wrd;
	const int			NARG = clang_Cursor_getNumArguments(h_parent);
	const aclspv_bool_t		c_has_this = aclutil_fn_has_this(h_parent);
	const int			NARG_TOTAL = NARG + (c_has_this ? 1 : 0);
	int				i;

	ctx->m_this = 0;

	aclemit_dtor_begin(h_ctx);

	if(!aclemit_dtor_push(h_ctx))
		return CXChildVisit_Break;

	p_sym = aclsym_fn_find(h_parent, ctx);

	/** a body this translation unit did not declare (an inline body in an
	 * included header): not a symbol, nothing to emit here. */
	unless(p_sym) {
		ctx->m_work_nxt = aclemit_root;
		return CXChildVisit_Continue;
	}

	/** the cursor arena and its root label were bootstrapped by
	 * aclemit_root; do not reset them here. */

	/** the emitting context builds its own signature: the gather's id lives
	 * in the slave context and is only a comparison key. */
	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(
				h_parent
				, clang_getCursorType(h_parent)
				, ctx))
		return CXChildVisit_Break;

	ae2f_ifezerr(p_root_ty = aclty_find(id_fn_ty, ctx))
		return CXChildVisit_Break;

	id_ret = p_root_ty->m_info.m_fn.m_ret_id;

	/** the symbol carries the emitted identity for call sites. */
	p_sym->m_id = ctx->m_id++;

	/** name the symbol when the flag asks for it. */
	ae2f_ifnezerr(aclemit_fndecl_sym_name(p_sym, ctx))
		return CXChildVisit_Break;

	/** the attribute half of the control mask is a property of the
	 * declaration, read lazily here. the purity half can only be answered
	 * after the body is carved, so the word is patched below. */
	c_fnctrl = aclattr_get_fnctrl(h_parent);

	/** where this OpFunction's control word lands:
	 * [op][result type][result id][control][function type]. it is the slot
	 * the instruction is written into; the parameters emitted next advance
	 * m_fndef past it. */
	c_fn_wrd = ctx->m_count.m_fndef;

	ae2f_ifezerr(ctx->m_count.m_fndef = aclutil_emitx_5(
				&ctx->m_section.m_fndef
				, ctx->m_count.m_fndef
				, SpvOpFunction
				, id_ret
				, p_sym->m_id
				, c_fnctrl
				, id_fn_ty))
		return CXChildVisit_Break;

	for(i = 0; i < NARG_TOTAL; ++i) {
		const int		c_idx_arg = c_has_this ? i - 1 : i;
		CXCursor		c_parm	= clang_getNullCursor();
		aclspv_bool_t		c_ref_param = 0;
		aclty_t* ae2f_restrict	p_arg;
		aclspv_id_t		id_parm;
		aclspv_id_t		id_reg;
		aclcur_t* ae2f_restrict	p_reg;

		/** the parameter type is taken from the interned signature, so the
		 * emitted parameter always matches the OpTypeFunction. */
		ae2f_ifezerr(p_root_ty = aclty_find(id_fn_ty, ctx))
			return CXChildVisit_Break;

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_root_ty, (aclspv_wrd_t)i, ctx))
			return CXChildVisit_Break;

		if(c_idx_arg >= 0) {
			const CXType TY_PRM = clang_getArgType(
					clang_getCursorType(h_parent), (unsigned)c_idx_arg);

			c_parm = clang_Cursor_getArgument(h_parent, (unsigned)c_idx_arg);
			c_ref_param = (aclspv_bool_t)(
					TY_PRM.kind == CXType_LValueReference
					|| TY_PRM.kind == CXType_RValueReference);
		}

		/** a pointer parameter into storage the function does not own
		 * (StorageBuffer, Uniform, Workgroup, ...) travels as a pointer
		 * argument: SPIR-V admits that through variable pointers. request
		 * the StorageBuffer form the annotations produce. */
		{
			aclty_t* ae2f_restrict p_pty = aclty_find(
					p_arg->m_info.m_fn_arg.m_prm_id
					, ctx
					);

			if(p_pty
					&& p_pty->m_class == ACLTY_PTR
					&& p_pty->m_info.m_ptr.m_storage
						!= SpvStorageClassFunction) {
				ae2f_ifezerr(aclutil_add_extension_by_flag(
							ctx, "SPV_KHR_variable_pointers"))
					return CXChildVisit_Break;

				ae2f_ifezerr(aclutil_add_capability_by_flag(
							ctx
							, SpvCapabilityVariablePointersStorageBuffer))
					return CXChildVisit_Break;
			}
		}

		id_parm = ctx->m_id++;

		ae2f_ifezerr(ctx->m_count.m_fndef = aclutil_emitx_3(
					&ctx->m_section.m_fndef
					, ctx->m_count.m_fndef
					, SpvOpFunctionParameter
					, p_arg->m_info.m_fn_arg.m_prm_id
					, id_parm))
			return CXChildVisit_Break;

		/** the parameter's value node: its id IS the OpFunctionParameter
		 * result. it is left unbound (null cursor) because it only feeds
		 * the variable below — a body reference must resolve to something
		 * assignable. */
		ae2f_ifezerr(id_reg = aclcur_cfg_reg(
					clang_getNullCursor()
					, p_arg->m_info.m_fn_arg.m_prm_id
					, SpvStorageClassFunction
					, 0
					, ctx))
			return CXChildVisit_Break;

		ae2f_ifezerr(p_reg = aclcur_get_node(id_reg, ctx))
			return CXChildVisit_Break;

		p_reg->m_id = id_parm;
		p_reg->m_val = id_parm;
		p_reg->m_flag0 |= aclcur_flag0(ACLCUR_FLAG0_VALUE)
			| aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM);

		/** the implicit object pointer has no ParmDecl to bind: keep the
		 * parameter value register as the method context. */
		if(c_has_this && i == 0) {
			/** the parameter value id is the node's identity after the
			 * m_id patch above; the cursor arena is keyed on it. */
			ctx->m_this = id_parm;
			continue;
		}

		/** a reference is a pointer VALUE, not an object to allocate: bind
		 * the value register straight to the ParmDecl so `o.x` resolves to
		 * the pointer and no pointer-to-pointer OpVariable is emitted. */
		if(c_ref_param) {
			p_reg->m_cur = c_parm;
			continue;
		}

		/** the variable the body refers to. every parameter gets one: an
		 * assignment is then an ordinary store the graph already knows how
		 * to handle (SSA definition, phi at a join, memory when the address
		 * escapes) instead of a fragile scan of the AST. the initial value
		 * is the parameter, defined at the entry. */
		{
			const aclcur_reg_t REGIN = aclcur_regvar_in();
			aclspv_id_t id_var;

			ae2f_ifezerr(id_var = aclcur_cfg_reg(
						c_parm
						, p_arg->m_info.m_fn_arg.m_prm_id
						, SpvStorageClassFunction
						, &REGIN
						, ctx))
				return CXChildVisit_Break;

			ae2f_ifezerr(aclcur_store_reg(id_var, id_parm, ctx))
				return CXChildVisit_Break;

			/** a by-value class parameter is a live object in this
			 * function: it is destroyed at function exit like any local. */
			if(!aclemit_dtor_reserve(
						c_parm
						, id_var
						, p_arg->m_info.m_fn_arg.m_prm_id
						, ctx))
				return CXChildVisit_Break;
		}
	}

	/** the root label's m_ty is the return type the body's return path
	 * evaluates against. aclemit_root bootstrapped it as void. */
	ae2f_ifezerr(p_root = aclcur_get_node_idx(ACLCUR_IDX_ROOT, ctx))
		return CXChildVisit_Break;

	p_root->m_ty = id_ret;

	{
		aclspv_bool_t			c_is_const = 0;
		aclspv_bool_t			c_is_pure = 0;
		enum CXChildVisitResult		ret;

		ret = aclemit_fndecl_bodycfg(h_cur, h_ctx, &c_is_const, &c_is_pure);

		aclctx_err_check(ctx)
			return ret;

		/** the body is the truth: the analysed answer replaces the declared
		 * promise gathered from the attributes. */
		p_sym->m_flag0 &= (aclspv_wrd_t)~(
				aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST)
				| aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE));

		if(c_is_const)
			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST);
		if(c_is_pure)
			p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE);

		p_sym->m_flag0 |= aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED);

		/** the body answered the purity question: fold it into the control
		 * word that was written before the body existed. */
		{
			aclspv_wrd_t* ae2f_restrict p_wrds
				= aclutil_get_wrd_of_vec(&ctx->m_section.m_fndef);

			/** SPIR-V Const implies Pure. the analysis answers the two
			 * questions independently (const: reads no external state,
			 * pure: writes none), so a body that writes external state
			 * gets neither bit. */
			if(p_sym->m_flag0
					& aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE)) {
				p_wrds[c_fn_wrd + 3] |= (aclspv_wrd_t)
					SpvFunctionControlPureMask;

				if(p_sym->m_flag0
						& aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST))
					p_wrds[c_fn_wrd + 3] |= (aclspv_wrd_t)
						SpvFunctionControlConstMask;
			}
		}

		return ret;
	}
}
