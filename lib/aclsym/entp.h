/** @file aclsym/entp.h
 *
 * pre-emit entry-point symbols: what the entry-point emitter needs to know
 * about a function before its body is carved.
 * */

#ifndef aclsym_entp_h
#define aclsym_entp_h

#include <aclspv.h>
#include <aclsym/fn.h>

typedef struct {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_idx_push_constant;
	aclspv_wrd_t	m_descriptor_set;

	/** short handle for the declaration: clang_hashCursor.
	 *
	 * no CXCursor is stored, for the same reason aclsym_fn_t stores a mark: the
	 * tree belongs to the caller. this lets a later walk of the same translation
	 * unit recognise an entry point that is already in the module, instead of
	 * emitting a second OpEntryPoint for it. */
	aclspv_wrd_t	m_mark;

	/** enum ACLSYM_FN_FLAG0_: CONST and PURE only. an entry point is not
	 * a call target, so it has no function symbol and its purity snapshot
	 * lives here instead. */
	aclspv_wrd_t	m_flag0;
} aclsym_entp_t;

#endif
