/** @file expr.c */

#include "./expr.h"

#include <aclspv/spvty.h>

#include <util/op.h>
#include <util/scale.h>
#include <util/emitx.h>

ACLSPV_ABI_IMPL enum EMIT_EXPR_ aclspv_get_expr2(
		aclspv_id_t* ae2f_restrict const	ret_id
		, aclspv_id_t* ae2f_restrict const	r_type_opt
		, const CXCursor			c_cur
		, util_literal* ae2f_restrict		wr_literal_opt
		, h_util_ctx_t				h_ctx
		) {
	uptr BUF[6 + (((sizeof(util_literal) + sizeof(uptr) - 1) / sizeof(uptr)))];
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;

	BUF[0]		= (uptr)h_ctx;			/* context. must be pointer to context. */
	BUF[1]		= 0;				/* stack count. must be 0. */
	BUF[2]		= 0;				/* result value id */
	BUF[3]		= UTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	BUF[4]		= 0;				/* type casted */
	BUF[5]		= 0;				/* count for operand */

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v0, 1 << 10);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v1, 1 << 10);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v2, 1 << 10);

	ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(init_scale(&h_ctx->m_tmp.m_v1
				, count_to_sz(UTIL_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(util_opbincfg_extra)
				))
		return EMIT_EXPR_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_tmp.m_v1, *get_last_scale_from_vec(&h_ctx->m_tmp.m_v1));
	ae2fpat_assertassume(SCALE_BOOT);
	ae2fpat_assertassume(!get_last_scale_from_vec(&h_ctx->m_tmp.m_v1)->m_id);

	SCALE_BOOT[UTIL_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[UTIL_OPBINCFG_MASKLITERAL]		= UTIL_LITERAL_MASK_LITERAL;
	SCALE_BOOT[UTIL_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[UTIL_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[UTIL_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[UTIL_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

	((umax* ae2f_restrict)h_ctx->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1], (void*)(SCALE_BOOT + 1))
		);

	if(aclexpr_visit(c_cur, c_cur, BUF) == CXChildVisit_Break)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;


	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "Constant flag after first emit_expr %u[%p]\n"
				, SCALE_BOOT[1], (void*)(SCALE_BOOT + 1))
		);

	clang_visitChildren(c_cur, aclexpr_visit, BUF);

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "BUF[3]: %u\n", (unsigned)BUF[UTIL_OPBINCFG_RESTYID]));

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "Type returned (BUF[4]): %u\n", (unsigned)BUF[UTIL_OPBINCFG_RESID]));

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "ID Required(aka BUF[2]): %u\n", (unsigned)BUF[2]));

	if(wr_literal_opt) 
		*wr_literal_opt = INIT_EXTRA->m_literal;

	if(r_type_opt)
		*r_type_opt = (aclspv_id_t)BUF[2];

	ae2fpat_assertassume_strict(ret_id);
	*ret_id = (aclspv_wrdcount_t)BUF[2];

	return		BUF[3] & UTIL_LITERAL_LITERAL ? EMIT_EXPR_SUCCESS_LITERAL
		: 	BUF[3] & UTIL_LITERAL_CONSTANT ? EMIT_EXPR_SUCCESS_CONSTANT
		: 	EMIT_EXPR_SUCCESS;

#undef	INIT_EXTRA
}

enum EMIT_EXPR_ aclspv_get_expr_btcast(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_util_ctx_t h_ctx
		) 
{
	uptr BUF[6 + (((sizeof(util_literal) + sizeof(uptr) - 1) / sizeof(uptr)))];
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;

	BUF[0]		= (uptr)h_ctx;			/* context. must be pointer to context. */
	BUF[1]		= 0;				/* stack count. must be 0. */
	BUF[2]		= 0;				/* result value id */
	BUF[3]		= UTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	BUF[4]		= 0;				/* type casted */
	BUF[5]		= 0;				/* count for operand */

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v0, 1 << 10);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v1, 1 << 10);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v2, 1 << 10);

	ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(init_scale(&h_ctx->m_tmp.m_v1
				, count_to_sz(UTIL_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(util_opbincfg_extra)
				))
		return EMIT_EXPR_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_tmp.m_v1, *get_last_scale_from_vec(&h_ctx->m_tmp.m_v1));
	assert(SCALE_BOOT);
	assert(!get_last_scale_from_vec(&h_ctx->m_tmp.m_v1)->m_id);

	SCALE_BOOT[UTIL_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[UTIL_OPBINCFG_MASKLITERAL]		= UTIL_LITERAL_MASK_LITERAL;
	SCALE_BOOT[UTIL_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[UTIL_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[UTIL_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[UTIL_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

#undef	INIT_EXTRA

	((umax* ae2f_restrict)h_ctx->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf
			, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1]
				, (void*)(SCALE_BOOT + 1))
		);

	if(aclexpr_visit(c_cur, c_cur, BUF) == CXChildVisit_Break)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;


	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "Constant flag after first emit_expr %u[%p]\n"
				, SCALE_BOOT[1], (void*)(SCALE_BOOT + 1))
		);

	clang_visitChildren(c_cur, aclexpr_visit, BUF);

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "BUF[3]: %u\n", (unsigned)BUF[3]));

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "Type returned (BUF[4]): %u\n", (unsigned)BUF[4]));

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "ID Required(aka BUF[2]): %u\n", (unsigned)BUF[2]));

	/** bitcast is not required when two types are exactly same */
	unless(BUF[4] != c_type_req)
		return	aclexpr_arithmetic_cast(
				(aclspv_wrdcount_t)BUF[2]
				, (aclspv_wrd_t)BUF[4]
				, (aclspv_wrd_t)BUF[3]
				, *(util_literal* ae2f_restrict)(void* ae2f_restrict)(BUF + 6)
				, ret_id
				, c_type_req
				, wr_literal_opt
				, h_ctx);

	ae2f_expected_but_else(aclid_is_number(c_type_req) 
			&& aclid_is_number((aclspv_wrd_t)BUF[4])) {
		return EMIT_EXPR_NOT_THE_CASE;
	}

	if(BUF[3] & UTIL_LITERAL_LITERAL) {
		if(wr_literal_opt)
			*wr_literal_opt = *(util_literal* ae2f_restrict)(void* ae2f_restrict)(BUF + 6);

		return aclexpr_arithmetic_cast_literal(
				c_type_req
				, *(util_literal* ae2f_restrict)(void*)(&BUF[6])
				, ret_id
				, c_type_req
				, wr_literal_opt
				, h_ctx
				);
	}

	{
		unsigned	BWIDTH;
		if((BWIDTH = aclid_get_bit_width(c_type_req)) != aclid_get_bit_width((aclspv_wrd_t)BUF[4]))
		{
			const aclspv_id_t TYPE = aclid_number_cvtbitcount(
					(aclspv_wrd_t)BUF[4]
					, BWIDTH
					);

			aclspv_id_t VAL;

			switch(aclexpr_arithmetic_cast_non_literal(
						(aclspv_id_t)BUF[2]
						, (aclspv_id_t)BUF[4]
						, (aclspv_id_t)BUF[3]
						, &VAL
						, TYPE
						, h_ctx))
			{
				default:
				case EMIT_EXPR_NOT_THE_CASE:
					return EMIT_EXPR_NOT_THE_CASE;
				case EMIT_EXPR_FAILURE:
					return EMIT_EXPR_FAILURE;

				case EMIT_EXPR_SUCCESS:
					BUF[3] = 0;
					ae2f_fallthrough;
				case EMIT_EXPR_SUCCESS_CONSTANT:
					BUF[3] &= 1;
					ae2f_fallthrough;
				case EMIT_EXPR_SUCCESS_LITERAL:
					BUF[2] = VAL;
			}
		}
	}


	ae2f_expected_but_else(aclid_mk_default_id(c_type_req, h_ctx)) {
		return EMIT_EXPR_FAILURE;
	}

	ae2f_expected_but_else(h_ctx->m_count.m_fnimpl = util_emitx_4(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, SpvOpBitcast
				, c_type_req
				, *ret_id = h_ctx->m_id
				, (aclspv_wrd_t)BUF[2]
				)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_EXPR;
		return EMIT_EXPR_FAILURE;
	}

	++h_ctx->m_id;

	return EMIT_EXPR_SUCCESS;
}

enum EMIT_EXPR_ aclspv_get_expr(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_util_ctx_t h_ctx
		) {
	uptr BUF[6 + (((sizeof(util_literal) + sizeof(uptr) - 1) / sizeof(uptr)))];
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;

	BUF[0]		= (uptr)h_ctx;			/* context. must be pointer to context. */
	BUF[1]		= 0;				/* stack count. must be 0. */
	BUF[2]		= 0;				/* result value id */
	BUF[3]		= UTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	BUF[4]		= 0;				/* type casted */
	BUF[5]		= 0;				/* count for operand */

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v0, 1 << 10);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v1, 1 << 10);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, h_ctx->m_tmp.m_v2, 1 << 10);

	ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(init_scale(&h_ctx->m_tmp.m_v1
				, count_to_sz(UTIL_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(util_opbincfg_extra)
				))
		return EMIT_EXPR_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_tmp.m_v1, *get_last_scale_from_vec(&h_ctx->m_tmp.m_v1));
	assert(SCALE_BOOT);
	assert(!get_last_scale_from_vec(&h_ctx->m_tmp.m_v1)->m_id);

	SCALE_BOOT[UTIL_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[UTIL_OPBINCFG_MASKLITERAL]		= UTIL_LITERAL_MASK_LITERAL;
	SCALE_BOOT[UTIL_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[UTIL_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[UTIL_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[UTIL_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

#undef	INIT_EXTRA

	((umax* ae2f_restrict)h_ctx->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf
			, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1]
				, (void*)(SCALE_BOOT + 1))
		);

	if(aclexpr_visit(c_cur, c_cur, BUF) == CXChildVisit_Break)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;


	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "Constant flag after first emit_expr %u[%p]\n"
				, SCALE_BOOT[1], (void*)(SCALE_BOOT + 1))
		);

	clang_visitChildren(c_cur, aclexpr_visit, BUF);

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "BUF[3]: %u\n", (unsigned)BUF[3]));

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "Type returned (BUF[4]): %u\n", (unsigned)BUF[4]));

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "ID Required(aka BUF[2]): %u\n", (unsigned)BUF[2]));

	return	aclexpr_arithmetic_cast(
			(aclspv_wrdcount_t)BUF[2]
			, (aclspv_wrd_t)BUF[4]
			, (aclspv_wrd_t)BUF[3]
			, *(util_literal* ae2f_restrict)(void* ae2f_restrict)(BUF + 6)
			, ret_id
			, c_type_req
			, wr_literal_opt
			, h_ctx);
}

