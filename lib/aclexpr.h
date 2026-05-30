/** @file expr.h */

#ifndef	aclexpr_h
#define	aclexpr_h
#include <util/literal.h>

typedef enum EMIT_EXPR_ {

	EMIT_EXPR_FAILURE,

	/** succeed, value was not constant */
	EMIT_EXPR_SUCCESS,

	/** succeed, value was constant. */
	EMIT_EXPR_SUCCESS_CONSTANT,

	/** succeed, value was literal. */
	EMIT_EXPR_SUCCESS_LITERAL,

	EMIT_EXPR_NOT_THE_CASE
} e_emit_expr_bin_1_t;

typedef util_literal emit_expr_literal;

enum EMIT_EXPR_ aclexpr_arithmetic_cast_non_literal(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const B_util_literal_t	c_flag_literal,
		aclspv_id_t* ae2f_restrict		r_newid,
		aclspv_id_t				c_new_type,
		const h_aclspv_ctx_t			h_ctx
		);

enum EMIT_EXPR_ aclexpr_arithmetic_cast(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const B_util_literal_t	c_flag_literal,
		const emit_expr_literal	c_old_literal,

		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t		c_new_type,
		emit_expr_literal* ae2f_restrict wr_new_literal_opt,
		const h_aclspv_ctx_t	h_ctx
		);

enum EMIT_EXPR_ aclexpr_arithmetic_cast_literal(
		const aclspv_id_t	c_old_type,
		const emit_expr_literal	c_old_literal,
		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t			c_new_type,
		emit_expr_literal* ae2f_restrict	wr_new_literal_opt,
		const h_aclspv_ctx_t		h_ctx
		);

typedef enum EMIT_EXPR_ fn_aclspv_get_expr_t(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

enum EMIT_EXPR_ aclexpr_eval_btcast(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

enum EMIT_EXPR_ aclexpr_eval(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

enum EMIT_EXPR_ aclspv_get_expr2(
		aclspv_id_t* ae2f_restrict const ret_id
		, aclspv_id_t* ae2f_restrict const r_type_opt
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

struct aclexpr_visit_data {
	h_aclspv_ctx_t		m_ctx;
	aclspv_wrdcount_t	m_exprcount;
	aclspv_id_t		m_id_ret;
	aclspv_wrd_t		m_literal_flag;
	aclspv_id_t		m_tyid_current;
	aclspv_id_t		m_num_cmdcounter;
	util_literal		m_ret_literal;
};

enum CXChildVisitResult aclexpr_visit(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData rdwr_data
		);


#endif
