#include "./shuffle.h"

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <aclty.h>
#include <aclmem.h>
#include <aclattr/getkinder.h>

#include <util/scale.h>
#include <util/visitfromroot.h>

/** true when `c_cur` is a `__builtin_shufflevector` surfaced as an
 * UnexposedExpr. */
aclspv_bool_t aclexpr_shuffle_detect(CXCursor c_cur) {
	CXCursor			CH[3];
	struct aclattr_getkinder	K;

	if(c_cur.kind != CXCursor_UnexposedExpr)
		return 0;

	K.m_cursor	= CH;
	K.m_res		= CXChildVisit_Continue;
	K.m_num_wanted	= 3;
	K.m_num_grepped	= 0;

	(void)clang_visitChildren(c_cur, s_aclattr_getkinder_visit, &K);

	if(K.m_num_grepped < 3)
		return 0;

	/** after the two vector operands come the literal components. */
	return CH[2].kind == CXCursor_IntegerLiteral;
}

/** `__builtin_shufflevector(v1, v2, i0, i1, ...)`.
 *
 * the expression is a CXCursor_UnexposedExpr whose children are the two
 * vectors followed by one constant index per result component. the indices
 * are not expressions to evaluate, so they must stay out of the operand
 * stream: this handler pushes a two-operand scale and visits only the two
 * vector subtrees. the carve recovers the indices from the stashed operator
 * cursor.
 * */
aclexpr_visit_lbl_t aclexpr_visit_opn_shuffle(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	x_scale* ae2f_restrict	NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict VEC;
	aclexpr_opbincfg_extra* ae2f_restrict EXTRAS;
	aclspv_id_t		TY;
	CXCursor		CH[2];
	struct aclattr_getkinder	K;

	unless(aclexpr_shuffle_detect(c_cur))
		return c_lbl_pass;

	TY = aclutil_mk_cxtp_by_type2(
			clang_getCanonicalType(clang_getCursorType(c_cur))
			, CTX);
	ae2f_ifezerr(TY) return c_lbl_fail;

	K.m_cursor	= CH;
	K.m_res		= CXChildVisit_Continue;
	K.m_num_wanted	= 2;
	K.m_num_grepped	= 0;

	(void)clang_visitChildren(c_cur, s_aclattr_getkinder_visit, &K);

	ae2f_ifnezerr_strict(K.m_num_grepped != 2)
		return c_lbl_fail;

	NEW_SCALE = mk_scale_from_vec(
			&CTX->m_tmp.m_v1
			, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 2)
			+ (sizeof(aclexpr_opbincfg_extra) * 3)
			);
	ae2f_ifezerr(NEW_SCALE) return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, &CTX->m_tmp.m_v1, *NEW_SCALE);
	ae2f_ifezerr(VEC) return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 2;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OPN_SHUFFLE;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= TY;

	/** extras[0..1] are the operand slots the cascade fills; extras[2] is
	 * ours and survives: the operator cursor, for the carve's mask read. */
	EXTRAS = (void* ae2f_restrict)(&VEC[ACLEXPR_OPBINCFG_END + 2]);
	aclmemzero(EXTRAS, sizeof(aclexpr_opbincfg_extra) * 3);
	EXTRAS[2].m_cursor = c_cur;

	/** the scale is pushed by mk_scale_from_vec; the matching command
	 * counter is what OP_RECURSE would have pushed for a normal operator. */
	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&CTX->m_tmp.m_v2
				, rdwr_data->m_num_cmdcounter + 1
				, sizeof(aclexpr_cmdcounterstckelm)
				, ACLSPV_COMPILE_ALLOC_FAILED_EXPR))
		return c_lbl_fail;

	(void)++rdwr_data->m_num_cmdcounter;
	((aclexpr_cmdcounterstckelm* ae2f_restrict)
	 CTX->m_tmp.m_v2.m_p)
		[rdwr_data->m_num_cmdcounter - 1].m_count_oprnd = 0;

	/** operand 0. a Break may be a normal stop after the operand carved. */
	(void)aclutil_visitfromroot(CH[0], aclexpr_visit, rdwr_data);
	if(CTX->m_err)
		return c_lbl_fail;

	/** operand 1 completes the scale and carves it. */
	(void)aclutil_visitfromroot(CH[1], aclexpr_visit, rdwr_data);
	if(CTX->m_err)
		return c_lbl_fail;

	return c_lbl_success;
}
