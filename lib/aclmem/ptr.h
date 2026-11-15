#ifndef	aclmem_ptr_h
#define aclmem_ptr_h

/* cppcheck-suppress misra-c2012-14.2 */
#define	aclmem_ptr_t(ty)	\
	typedef union {	\
		aclspv_wrd_t	m_a[(sizeof(ty*) + sizeof(aclspv_wrd_t) - 1u) / sizeof(aclspv_wrd_t)];\
		ty*		m_b;\
	} ty##_aclmem_ptr_t

/* cppcheck-suppress misra-c2012-14.2 */
#define aclmem_ptr_to_wrd(ty)\
	ae2f_inline_f static ae2f_ccconst ty##_aclmem_ptr_t ty##_aclmem_ptr_to_wrd(ty* p) {\
		ty##_aclmem_ptr_t a;\
		a.m_b = (p);\
		return a;\
	} ae2f_inline_f static ae2f_ccconst ty##_aclmem_ptr_t ty##_aclmem_ptr_to_wrd(ty* p)

/* cppcheck-suppress misra-c2012-14.2 */
#define aclmem_wrd_to_ptr(ty)\
	ae2f_inline_f static ae2f_ccpure\
	ty* ty##_aclmem_wrd_to_ptr(aclspv_wrd_t ae2fcc_arg_sarr_restrict(arr, (sizeof(ty*) + sizeof(aclspv_wrd_t) - 1u))) {\
		ty##_aclmem_ptr_t a; int i;\
		for(i = sizeof(a.m_a) / sizeof(a.m_a[0]); i --> 0; ) { \
			a.m_a[i] = arr[i];\
		}\
		return a.m_b;\
	} ty* ty##_aclmem_wrd_to_ptr(aclspv_wrd_t ae2fcc_arg_sarr_restrict(arr, (sizeof(ty*) + sizeof(aclspv_wrd_t) - 1u)))

/* cppcheck-suppress misra-c2012-14.2 */
#define	aclmem_ptr(ty) aclmem_ptr_t(ty); aclmem_ptr_to_wrd(ty); aclmem_wrd_to_ptr(ty)

#endif
