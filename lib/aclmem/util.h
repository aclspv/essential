#ifndef	aclmem_util_h
#define aclmem_util_h

#include <aclspv.h>
#include "../aclmem.h"
#include <ae2f/cc/profiles.h>
#include "./util/varch.h"

typedef aclspv_wrd2_t b64;
typedef aclspv_wrd_t b32;

ae2f_unused typedef aclspv_wrd2_t b64_v[sizeof(aclspv_wrd2_t) == (8) ? 1 : -1];
ae2f_unused typedef aclspv_wrd_t b32_v[sizeof(aclspv_wrd_t) == (4) ? 1 : -1];

#if	b128_avail
typedef aclspv_wrd_t b128 __attribute__((vector_size(16)));
#else
typedef struct { uint32_t m_0, m_1, m_2, m_3; } b128;
#endif

#if	b256_avail
typedef aclspv_wrd_t b256 __attribute__((vector_size(32)));
#else
typedef struct { uint32_t m_0, m_1, m_2, m_3, m_4, m_5, m_6, m_7; } b256;
#endif

ae2f_unused typedef aclspv_wrd_t b128_v[sizeof(b128) == 0x10 ? 1 : -1];
ae2f_unused typedef aclspv_wrd_t b256_v[sizeof(b256) == 0x20 ? 1 : -1];

#if	ae2f_stdc_v >= 199900L
#define	ZERO128	((b128) {0, 0, 0, 0})
#define	ZERO256	((b256) {0, 0, 0, 0, 0, 0, 0, 0})
#else
ae2f_inline_f static ae2f_ccconst b128 s_z128(void) {
	const b128 Z = { 0, 0, 0, 0 };
	return Z;
}
#define	ZERO128	s_z128()
ae2f_inline_f static ae2f_ccconst b256 s_z256(void) {
	const b256 Z = { 0, 0, 0, 0, 0, 0, 0, 0 };
	return Z;
}
#define	ZERO256	s_z256()
#endif


#define	ZERO64	0
#define	ZERO32	0
#define	ZERO16	0
#define	ZERO8	0

#endif
