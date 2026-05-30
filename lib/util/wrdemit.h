/** @file wrdemit.h @brief word emission utilities */

#ifndef build_wrdemit_h
#define build_wrdemit_h


#include <ae2f/c90/Limits.h>
#include <stddef.h>
#include <aclspv.h>
#include <.aclspv/vec.h>

#include "./wrd.h"

#include <assert.h>
#include <string.h>

#include <ae2f/bll.h>

/** size_t for spir-v format */
typedef	aclspv_wrdcount_t	aclspv_sz_t;

/** 
 * @def 	sz_to_count 
 * @brief	byte size to word count
 * */
#define sz_to_count(c_sz)	\
	(((aclspv_sz_t)(c_sz) + (aclspv_sz_t)(sizeof(aclspv_wrd_t)) - 1) / (aclspv_sz_t)(sizeof(aclspv_wrd_t)))


/**
 * @def		count_to_sz
 * @brief	word count to byte size
 * */
#define count_to_sz(c_count)	((aclspv_sz_t)(((aclspv_sz_t)(c_count)) * (aclspv_sz_t)(sizeof(aclspv_wrd_t))))

/**
 * @def		get_wrd_of_vec
 * @brief	get word buffer from vector
 * */
#define get_wrd_of_vec(vec)	((aclspv_wrd_t* ae2f_restrict)((vec)->m_p))

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
 

	while(c_wrdcount + 1 && c_wrdcount >= sz_to_count(h_wrds->m_sz)) {
		const size_t	NEWSIZE = h_wrds->m_sz ? (h_wrds->m_sz) << 1 : (size_t)count_to_sz(c_wrdcount + 1);
		_aclspv_grow_vec_with_copy(
				_aclspv_malloc, _aclspv_free, _aclspv_memcpy 
				, L_new, *h_wrds
				, NEWSIZE
				);

		ae2f_ifezerr(h_wrds->m_p) return 0;
	}

	CND = ae2f_bll_from_bool1(uptr, ae2f_expected(h_wrds->m_p && (sz_to_count(h_wrds->m_sz) <= UINT_MAX)));

	*ae2f_bll_sel_ptr(
			aclspv_wrd_t* ae2f_restrict
			, CND
			, &get_wrd_of_vec(h_wrds)[c_wrdcount]
			, &DUMMY) = c_wrd;

	return ae2f_bll_sel(
			aclspv_wrd_t
			, (aclspv_sz_t)CND
			, c_wrdcount + 1
			, 0u
			);
}

#define	mk_noprnds(c_num_opprm)	\
	((((aclspv_wrd_t)((c_num_opprm) + 1)) << 16) & (aclspv_wrd_t)ACLSPV_MASK_NOPRNDS)

#define	opcode_to_wrd(c_opcode, c_num_opprm)	\
	((aclspv_wrd_t)(((aclspv_wrd_t)(c_opcode) & ACLSPV_MASK_OPCODE) | mk_noprnds(c_num_opprm)))
/**
 * @def		emit_opcode
 * @brief	try emit opcode with num_opprm
 * */
#define	emit_opcode(h_wrds, c_wrdcount, c_opcode, c_num_opprm_opt)	\
	(aclutil_emit_wrd(h_wrds, c_wrdcount, opcode_to_wrd(c_opcode, c_num_opprm_opt)))

#define set_oprnd_count_for_opcode(cr_wrd, c_num_opprm)	\
	(cr_wrd) = opcode_to_wrd((cr_wrd & ACLSPV_MASK_OPCODE), ((aclspv_num_opprm_t)(c_num_opprm)))

#include <.aclspv/util/vcopr.h>

/**
 * @fn		emit_str
 * @brief	emit string with word size padded.
 * @returns	0 when failed.
 * */
ae2f_inline static aclspv_sz_t util_emit_str(
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

	_aclspv_grow_vec_with_copy(_aclspv_malloc, _aclspv_free, _aclspv_memcpy, L_new
			, *h_wrds, (size_t)count_to_sz(pad_wrds + c_wrdcount));

	ae2f_expected_but_else(h_wrds->m_p)	return 0;
	assert(count_to_sz(pad_wrds + c_wrdcount) <= (aclspv_sz_t)h_wrds->m_sz);

	memset(&get_wrd_of_vec(h_wrds)[c_wrdcount], 0, (size_t)count_to_sz(pad_wrds));
	_aclspv_memcpy(&get_wrd_of_vec(h_wrds)[c_wrdcount], rd_str, len);

	return c_wrdcount + pad_wrds;
}

#endif
