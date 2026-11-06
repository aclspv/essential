/** @file expr.h */

#ifndef	aclexpr_h
#define	aclexpr_h

#include <acldef.h>
#include <aclty/common.h>
#include <util/literal.h>

enum ACLEXPR_RET_ {
	ACLEXPR_RET_FAILURE,

	/** succeed, value was not constant */
	ACLEXPR_RET_SUCCESS,

	/** succeed, value was constant. */
	ACLEXPR_RET_SUCCESS_CONSTANT,

	/** succeed, value was literal. */
	ACLEXPR_RET_SUCCESS_LITERAL,

	ACLEXPR_RET_NOT_THE_CASE
};

typedef aclutil_literal aclexpr_literal;

enum ACLEXPR_RET_ aclexpr_arithmetic_cast_non_literal(
		const aclspv_vid_t	c_old_id,
		const aclty_id_t	c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		aclspv_vid_t* ae2f_restrict		r_newid,
		aclty_id_t				c_new_type,
		const h_aclspv_ctx_t			h_ctx
		);

enum ACLEXPR_RET_ aclexpr_arithmetic_cast(
		const aclspv_vid_t		c_old_id,
		const aclty_id_t		c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		const aclutil_literal		c_old_literal,

		aclspv_vid_t* ae2f_restrict	r_newid,
		aclty_id_t		c_new_type,
		aclutil_literal* ae2f_restrict wr_new_literal_opt,
		const h_aclspv_ctx_t	h_ctx
		);

enum ACLEXPR_RET_ aclexpr_bit_cast(
		const aclspv_vid_t	c_old_id,
		const aclty_id_t	c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		const aclutil_literal	c_old_literal,

		aclspv_vid_t* ae2f_restrict	r_newid,
		aclty_id_t		c_new_type,
		aclutil_literal* ae2f_restrict r_new_literal_opt,
		const h_aclspv_ctx_t	h_ctx
		);

enum ACLEXPR_RET_ aclexpr_arithmetic_cast_literal(
		const aclty_id_t	c_old_type,
		const aclutil_literal	c_old_literal,
		aclspv_vid_t* ae2f_restrict	r_newid,
		aclty_id_t			c_new_type,
		aclutil_literal* ae2f_restrict	wr_new_literal_opt,
		const h_aclspv_ctx_t		h_ctx
		);

typedef enum ACLEXPR_RET_ fn_aclspv_get_expr_t(
		aclspv_vid_t* ae2f_restrict const ret_id
		, const aclty_id_t c_type_req
		, const CXCursor c_cur
		, aclutil_literal* ae2f_restrict	r_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

enum ACLEXPR_RET_ aclexpr_eval_btcast(
		aclspv_vid_t* ae2f_restrict const ret_id
		, const aclty_id_t c_type_req
		, const CXCursor c_cur
		, aclutil_literal* ae2f_restrict	r_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

enum ACLEXPR_RET_ aclexpr_eval(
		aclspv_vid_t* ae2f_restrict const ret_id
		, const aclty_id_t c_type_req
		, const CXCursor c_cur
		, aclutil_literal* ae2f_restrict	r_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

enum ACLEXPR_RET_ aclspv_get_expr2(
		aclspv_vid_t* ae2f_restrict const ret_id
		, aclty_id_t* ae2f_restrict const r_type_opt
		, const CXCursor c_cur
		, aclutil_literal* ae2f_restrict	r_literal_opt
		, h_aclspv_ctx_t h_ctx
		);

struct aclexpr_visit_data {
	h_aclspv_ctx_t			m_ctx;
	aclspv_wrdcount_t		m_exprcount;
	aclspv_vid_t			m_id_ret;
	aclspv_wrd_t			m_literal_flag;
	aclty_id_t			m_tyid_current;
	aclspv_wrd_t			m_num_cmdcounter;
	aclutil_literal			m_ret_literal;

	/** short-circuit merge label: set by the bool1 before-visitor, consumed
	 * by the bool1 carver. expression-local; it is not the open block. */
	aclspv_id_t			m_id_branch;
};


typedef struct {
	aclspv_wrd_t	m_id_req;
} aclexprstck;

typedef struct {
	aclspv_wrd_t	m_count_oprnd;
} aclexpr_cmdcounterstckelm;

enum CXChildVisitResult aclexpr_visit(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData rdwr_data
		);


#endif
