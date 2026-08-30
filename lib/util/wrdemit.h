/** @file wrdemit.h @brief word emission utilities */

#ifndef build_wrdemit_h
#define build_wrdemit_h


#include <ae2f/c90/lmts.h>
#include <aclspv.h>
#include <aclmem.h>

#include <.aclspv/util/vcopr.h>

#include "./wrd.h"
#include "./log.h"

#include <assert.h>

#include <ae2f/bll.h>

/** size_t for spir-v format */
typedef	aclspv_wrdcount_t	aclspv_sz_t;

/** 
 * @def 	aclutil_sz_to_count 
 * @brief	byte size to word count
 * */
#define aclutil_sz_to_count(c_sz)	\
	(((aclspv_sz_t)(c_sz) + (aclspv_sz_t)(sizeof(aclspv_wrd_t)) - 1) / (aclspv_sz_t)(sizeof(aclspv_wrd_t)))


/**
 * @def		aclutil_count_to_sz
 * @brief	word count to byte size
 * */
#define aclutil_count_to_sz(c_count)	((aclspv_sz_t)(((aclspv_sz_t)(c_count)) * (aclspv_sz_t)(sizeof(aclspv_wrd_t))))

/**
 * @def		aclutil_get_wrd_of_vec
 * @brief	get word buffer from vector
 * */
#define aclutil_get_wrd_of_vec(vec)	((aclspv_wrd_t* ae2f_restrict)((vec)->m_p))

/**
 *
 * @fn		emit_wrd
 * @brief	try emitting `c_wrd` on `h_vec`
 * @return	new c_wrdcount available. 0 when failed allocating.
 * */
static ae2f_inline aclspv_sz_t aclutil_emit_wrd(
		x_aclspv_vec* ae2f_restrict const h_wrds, 
		const aclspv_sz_t c_wrdcount, 
		const aclspv_wrd_t c_wrd
		)
{
	aclspv_wrd_t	DUMMY;
	uptr		CND;


	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
			h_wrds
			, c_wrdcount + 1
			, sizeof(aclspv_wrd_t)
			, ACLSPV_COMPILE_ALLOC_FAILED_EMIT_WRD
			)) {
		err_puts("failed");
		return 0;
	}

	CND = ae2f_bll_from_bool1(uptr, ae2f_expected(h_wrds->m_p && (aclutil_sz_to_count(h_wrds->m_sz) <= UINT_MAX)));

	*ae2f_bll_sel_ptr(
			aclspv_wrd_t* ae2f_restrict
			, CND
			, &aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount]
			, &DUMMY) = c_wrd;

	return ae2f_bll_sel(
			aclspv_wrd_t
			, (aclspv_sz_t)CND
			, c_wrdcount + 1
			, 0u
			);
}

#define	aclspv_mk_noprnds(c_num_opprm)	\
	((((aclspv_wrd_t)((c_num_opprm) + 1)) << 16) & (aclspv_wrd_t)ACLSPV_MASK_NOPRNDS)

#define	aclspv_opcode_to_wrd(c_opcode, c_num_opprm)	\
	((aclspv_wrd_t)(((aclspv_wrd_t)(c_opcode) & ACLSPV_MASK_OPCODE) | aclspv_mk_noprnds(c_num_opprm)))
/**
 * @def		emit_opcode
 * @brief	try emit opcode with num_opprm
 * */
#define	emit_opcode(h_wrds, c_wrdcount, c_opcode, c_num_opprm_opt)	\
	(aclutil_emit_wrd(h_wrds, c_wrdcount, aclspv_opcode_to_wrd(c_opcode, c_num_opprm_opt)))


#define aclspv_get_oprnd_count_for_opcode(c_wrd)	\
	(((c_wrd) >> 16) & 0xFFFF)

#define set_oprnd_count_for_opcode(cr_wrd, c_num_opprm)	\
	(cr_wrd) = aclspv_opcode_to_wrd((cr_wrd & ACLSPV_MASK_OPCODE), ((aclspv_num_opprm_t)(c_num_opprm)))


#include <string.h>

/**
 * @fn		emit_str
 * @brief	emit string with word size padded.
 * @returns	0 when failed.
 * */
ae2f_inline static aclspv_sz_t aclutil_emit_str(
		x_aclspv_vec* ae2f_restrict const h_wrds, 
		const aclspv_sz_t c_wrdcount, 
		const char* ae2f_restrict const rd_str
		)
{
	/** string len: null included. */
	const size_t	len_		= strlen(rd_str) + 1;
	const aclspv_sz_t	len		= (len_ > (size_t)0xFFFFFFFF) ? 0 : (aclspv_sz_t)len_;
	const aclspv_sz_t	pad_wrds	= (len + 3) >> 2;

	ae2f_expected_but_else(len)		return 0;

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
			h_wrds
			, pad_wrds + c_wrdcount
			, sizeof(aclspv_wrd_t)
			, ACLSPV_COMPILE_ALLOC_FAILED_EMIT_WRD
			)) return 0;

	assert(aclutil_count_to_sz(pad_wrds + c_wrdcount) <= (aclspv_sz_t)h_wrds->m_sz);

	aclmemzero(&aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount], aclutil_count_to_sz(pad_wrds));
	aclmemcpy(&aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount], rd_str, len);

	return c_wrdcount + pad_wrds;
}

#endif
