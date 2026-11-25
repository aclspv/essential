#ifndef	__ACLSPV_MATH_NR
#define	__ACLSPV_MATH_NR	0
#endif

#include "./dtor.h"

#include <aclctx/err.h>

#include <aclexpr/carve/opn/call.h>
#include <aclexpr/carve/ptr.h>

#include <aclsym/fn.h>

#include <aclcur/common.h>
#include <aclcur/reg.h>

#include <aclty.h>
#include <aclval.h>
#include <aclcur/reg/view.h>

#include <aclutil/scale.h>
#include <aclemit/inline.h>
#include <aclattr/fnctrl.h>

/** one reserved local object: the register that owns the storage, the
 * emitted destructor id, and the destructor's interned function type. */
typedef struct {
	aclspv_id_t	m_reg;
	aclspv_id_t	m_fn;
	aclty_id_t	m_fn_ty;
	CXCursor	m_cur;
} acldtor_t;

/** the used count lives in the first word of each scale frame. */
static aclspv_wrd_t* s_frame(h_aclspv_ctx_t h_ctx) {
	h_scale_t S = get_last_scale_from_vec(&h_ctx->m_dtor_scope);
	return S ? get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_dtor_scope, *S) : 0;
}

static acldtor_t* s_records(aclspv_wrd_t* c_frame) {
	return (acldtor_t*)(void*)(c_frame + 1);
}

static enum CXChildVisitResult s_find_dtor(
		CXCursor		c,
		ae2f_unused CXCursor	p,
		CXClientData		rw
		) {
	CXCursor* ae2f_restrict r_dtor = rw;

	(void)p;

	if(clang_getCursorKind(c) != CXCursor_Destructor)
		return CXChildVisit_Continue;

	*r_dtor = c;
	return CXChildVisit_Break;
}

static CXCursor s_dtor_of(CXCursor c_var) {
	const CXType	TY = clang_getCanonicalType(clang_getCursorType(c_var));
	const CXCursor	DECL = clang_getTypeDeclaration(TY);
	CXCursor	DTOR = clang_getNullCursor();

	if(clang_Cursor_isNull(DECL))
		return DTOR;

	clang_visitChildren(DECL, s_find_dtor, &DTOR);
	return DTOR;
}

static aclspv_bool_t s_emit_one(const acldtor_t* c_rec, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t		p_reg;
	aclty_t* ae2f_restrict	p_fn;
	aclspv_id_t		id_obj;
	aclspv_id_t		id_call;
	aclspv_bool_t		is_pure = 0;

	ae2f_ifezerr(p_reg = aclcur_get_node(c_rec->m_reg, h_ctx))
		return 0;

	/** the reservation belongs to one object. a move or a by-value pass
	 * clears it, and then there is nothing to destroy here. */
	unless(p_reg->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_DTOR))
		return 1;

	aclexpr_ptr_taken(c_rec->m_reg, h_ctx);

	ae2f_ifezerr(id_obj = aclcur_reg_addr(c_rec->m_reg, h_ctx))
		return 0;

	/** a subobject is an OpAccessChain view; SPIR-V rejects it as an
	 * OpFunctionCall pointer argument, so inline the destructor body. only
	 * an inlinable destructor can be used for a subobject. */
	if(p_reg->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2)) {
		if(aclattr_is_noinline(c_rec->m_cur)
				|| !(aclattr_is_always_inline(c_rec->m_cur)
					|| clang_Cursor_isFunctionInlined(c_rec->m_cur))) {
			err_puts("[dtor] a subobject destructor must be inlinable");
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 0;
		}

		if(!aclemit_inline_fn(
					c_rec->m_cur
					, clang_getNullCursor()
					, &id_obj
					, 1
					, 0
					, h_ctx))
			return 0;
	}
	else {
		ae2f_ifezerr(p_fn = aclty_find(c_rec->m_fn_ty, h_ctx))
			return 0;

		ae2f_ifnezerr(aclexpr_call_make(
					c_rec->m_fn
					, p_fn->m_info.m_fn.m_ret_id
					, &id_obj
					, 1
					, &id_call
					, &is_pure
					, h_ctx))
			return 0;
	}

	p_reg = aclcur_get_node(c_rec->m_reg, h_ctx);

	if(p_reg)
		p_reg->m_flag0 &= (aclspv_wrd_t)~aclcur_flag0_reg(ACLCUR_FLAG0_REG_DTOR);

	return 1;
}

static aclspv_bool_t s_flush_last(h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t*		FRAME = s_frame(h_ctx);
	acldtor_t*		REC;
	aclspv_wrd_t		c_used;
	aclspv_wrd_t		i;

	unless(FRAME)
		return 0;

	c_used = FRAME[0];
	REC = s_records(FRAME);

	for(i = c_used; i --> 0; ) {
		unless(s_emit_one(&REC[i], h_ctx))
			return 0;
	}

	FRAME = s_frame(h_ctx);

	if(FRAME)
		FRAME[0] = 0;

	return 1;
}

void aclemit_dtor_begin(h_aclspv_ctx_t h_ctx) {
	h_scale_t S;

	if(!h_ctx)
		return;

	aclmem_fini(&h_ctx->m_dtor_scope);
	aclmem_init(&h_ctx->m_dtor_scope, h_ctx);

	S = init_scale(&h_ctx->m_dtor_scope, sizeof(aclspv_wrd_t));

	if(!S) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_VEC);
		return;
	}

	get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_dtor_scope, *S)[0] = 0;
}

void aclemit_dtor_end(h_aclspv_ctx_t h_ctx) {
	if(!h_ctx)
		return;

	(void)s_flush_last(h_ctx);
}

aclspv_bool_t aclemit_dtor_push(h_aclspv_ctx_t h_ctx) {
	h_scale_t S;

	ae2f_ifezerr(h_ctx)	return 0;

	S = mk_scale_from_vec(&h_ctx->m_dtor_scope, sizeof(aclspv_wrd_t));

	ae2f_ifezerr(S)	{
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_VEC);
		return 0;
	}

	get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_dtor_scope, *S)[0] = 0;
	return 1;
}

aclspv_bool_t aclemit_dtor_pop(h_aclspv_ctx_t h_ctx) {
	h_scale_t LAST, FIRST;

	ae2f_ifezerr(h_ctx)	return 0;

	unless(s_flush_last(h_ctx))
		return 0;

	LAST = get_last_scale_from_vec(&h_ctx->m_dtor_scope);
	FIRST = get_first_scale_from_vec(&h_ctx->m_dtor_scope);

	if(LAST && LAST != FIRST)
		(void)del_scale_from_vec_last(&h_ctx->m_dtor_scope);

	return 1;
}

aclspv_bool_t aclemit_dtor_flush_all(h_aclspv_ctx_t h_ctx) {
	h_scale_t LAST, FIRST;

	ae2f_ifezerr(h_ctx)	return 0;

	ae2f_ifezerr(h_ctx->m_dtor_scope.m_p)	return 1;

	LAST = get_last_scale_from_vec(&h_ctx->m_dtor_scope);
	FIRST = get_first_scale_from_vec(&h_ctx->m_dtor_scope);

	for(;;) {
		unless(s_flush_last(h_ctx))
			return 0;

		LAST = get_last_scale_from_vec(&h_ctx->m_dtor_scope);

		if(!LAST || LAST == FIRST)
			break;

		(void)del_scale_from_vec_last(&h_ctx->m_dtor_scope);
	}

	return 1;
}

static aclspv_bool_t s_reserve_own(
		CXCursor	c_var,
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	const CXCursor		DTOR = s_dtor_of(c_var);
	aclsym_fn_t* ae2f_restrict p_sym;
	aclspv_id_t		id_fn_ty;
	h_scale_t		LAST;
	aclspv_wrd_t*		FRAME;
	acldtor_t*		REC;
	aclspv_wrd_t		c_used;

	ae2f_ifezerr(h_ctx)	return 0;

	if(clang_Cursor_isNull(DTOR))
		return 1;	/** trivial type: nothing to reserve */

	p_sym = aclsym_fn_find(DTOR, h_ctx);

	if(!p_sym) {
		/** an implicitly-declared destructor has no body to call. */
		return 1;
	}

	if(!(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 0;
	}

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(
				DTOR, clang_getCursorType(DTOR), h_ctx)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 0;
	}

	LAST = get_last_scale_from_vec(&h_ctx->m_dtor_scope);

	if(!LAST) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_NOT_INIT_STATE);
		return 0;
	}

	FRAME = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_dtor_scope, *LAST);

	unless(FRAME) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_VEC);
		return 0;
	}

	c_used = FRAME[0];

	ae2f_ifnezerr_strict(grow_last_scale(
				&h_ctx->m_dtor_scope
				, (aclspv_wrd_t)(sizeof(aclspv_wrd_t)
					+ (c_used + 1) * sizeof(acldtor_t)))) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_VEC);
		return 0;
	}

	LAST = get_last_scale_from_vec(&h_ctx->m_dtor_scope);
	FRAME = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_dtor_scope, *LAST);
	REC = s_records(FRAME);

	REC[c_used].m_reg = c_reg;
	REC[c_used].m_fn = p_sym->m_id;
	REC[c_used].m_fn_ty = id_fn_ty;
	REC[c_used].m_cur = DTOR;

	FRAME[0] = c_used + 1;

	{
		h_aclcur_t p_reg = aclcur_get_node(c_reg, h_ctx);

		if(p_reg)
			p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_DTOR);
	}

	return 1;
}


struct s_field_visit {
	CXCursor	m_record;
	aclspv_id_t	m_reg;
	aclspv_wrd_t	m_storage;
	aclspv_wrd_t	m_idx;
	h_aclspv_ctx_t	m_ctx;
};

static aclspv_bool_t s_reserve_type(
		CXCursor	c_cur,
		CXType		c_ty,
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx);

static enum CXChildVisitResult s_field_dtor_visit(
		CXCursor		c_field,
		ae2f_unused CXCursor	c_par,
		CXClientData		rw
		) {
	struct s_field_visit* ae2f_restrict p = rw;
	CXType			TY;
	aclty_id_t		TY_ID;
	aclspv_id_t		IDX, VIEW;
	enum CXTypeKind		KIND;

	if(clang_getCursorKind(c_field) != CXCursor_FieldDecl)
		return CXChildVisit_Continue;

	TY = clang_getCanonicalType(clang_getCursorType(c_field));
	KIND = TY.kind;

	if(KIND != CXType_Record && KIND != CXType_ConstantArray) {
		++p->m_idx;
		return CXChildVisit_Continue;
	}

	ae2f_ifezerr(TY_ID = aclty_mk_by_cursor_default(c_field, TY, p->m_ctx)) {
		_aclctx_err(p->m_ctx, ACLSPV_COMPILE_CXTP_ZERO);
		return CXChildVisit_Break;
	}

	ae2f_ifezerr(IDX = aclutil_mk_constant_val_id(p->m_idx, p->m_ctx)) {
		_aclctx_err(p->m_ctx, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT);
		return CXChildVisit_Break;
	}

	ae2f_ifezerr(VIEW = aclcur_reg_view(
				c_field
				, p->m_reg
				, TY_ID
				, IDX
				, p->m_storage
				, ACLCUR_REGVIEW_STEP_MEMBER
				, p->m_ctx)) {
		_aclctx_err(p->m_ctx, ACLSPV_COMPILE_UNHANDLED);
		return CXChildVisit_Break;
	}

	if(!s_reserve_type(c_field, TY, VIEW, p->m_ctx))
		return CXChildVisit_Break;

	++p->m_idx;
	return CXChildVisit_Continue;
}

static aclspv_bool_t s_reserve_type(
		CXCursor	c_cur,
		CXType		c_ty,
		aclspv_id_t	c_reg,
		h_aclspv_ctx_t	h_ctx
		) {
	h_aclcur_t		p_reg;
	CXCursor		DECL;
	CXType			TY = clang_getCanonicalType(c_ty);

	p_reg = aclcur_get_node(c_reg, h_ctx);

	if(!p_reg)
		return 1;

	switch((aclspv_cxtykind_t)TY.kind) {
		default:
			return 1;

		case CXType_Record:
			{
				struct s_field_visit FV;

				DECL = clang_getTypeDeclaration(TY);

				if(clang_Cursor_isNull(DECL))
					return 1;

				FV.m_record	= DECL;
				FV.m_reg	= c_reg;
				FV.m_storage	= p_reg->m_d.m_reg.m_storage_class;
				FV.m_idx	= 0;
				FV.m_ctx	= h_ctx;

				clang_visitChildren(DECL, s_field_dtor_visit, &FV);

				aclctx_err_check(h_ctx)
					return 0;

				/** the object's own destructor runs before its members are
				 * destroyed, so reserve members first and the object last. */
				return s_reserve_own(DECL, c_reg, h_ctx);
			}

		case CXType_ConstantArray:
			{
				CXType		TYPE_ELM = clang_getArrayElementType(TY);
				const aclspv_swrd2_t c_num = clang_getNumElements(TY);
				const aclspv_wrd_t c_n = (aclspv_wrd_t)(c_num > 0xFFFF ? 0 : c_num);
				aclspv_wrd_t i;

				if(!c_n)
					return 1;

				for(i = 0; i < c_n; ++i) {
					aclty_id_t	TY_ID;
					aclspv_id_t	IDX, VIEW;

					ae2f_ifezerr(TY_ID = aclty_mk_by_cursor_default(
							c_cur, TYPE_ELM, h_ctx)) {
						_aclctx_err(h_ctx, ACLSPV_COMPILE_CXTP_ZERO);
						return 0;
					}

					ae2f_ifezerr(IDX = aclutil_mk_constant_val_id(
							(aclspv_wrd_t)i, h_ctx)) {
						_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT);
						return 0;
					}

					ae2f_ifezerr(VIEW = aclcur_reg_view(
							c_cur
							, c_reg
							, TY_ID
							, IDX
							, p_reg->m_d.m_reg.m_storage_class
							, ACLCUR_REGVIEW_STEP_ELEMENT
							, h_ctx)) {
						_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
						return 0;
					}

					if(!s_reserve_type(c_cur, TYPE_ELM, VIEW, h_ctx))
						return 0;
				}

				return 1;
			}
	}
}

aclspv_bool_t aclemit_dtor_reserve(
		CXCursor	c_var,
		aclspv_id_t	c_reg,
		aclty_id_t	c_ty,
		h_aclspv_ctx_t	h_ctx
		) {
	(void)c_ty;
	return s_reserve_type(c_var, clang_getCursorType(c_var), c_reg, h_ctx);
}

aclspv_bool_t aclemit_dtor_flush_to(
		h_aclspv_ctx_t	h_ctx,
		aclspv_wrd_t	c_base_id
		) {
	h_scale_t LAST;

	ae2f_ifezerr(h_ctx)	return 0;

	for(;;) {
		LAST = get_last_scale_from_vec(&h_ctx->m_dtor_scope);

		if(!LAST || LAST->m_id <= c_base_id)
			break;

		unless(s_flush_last(h_ctx))
			return 0;

		(void)del_scale_from_vec_last(&h_ctx->m_dtor_scope);
	}

	return 1;
}
