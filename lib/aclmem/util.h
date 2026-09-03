#ifndef	aclmem_util_h
#define aclmem_util_h

#include <aclspv.h>
#include "../aclmem.h"
#include <ae2f/cc/profiles.h>

typedef aclspv_wrd2_t b64;
typedef aclspv_wrd_t b32;

ae2f_unused typedef aclspv_wrd2_t b64_v[sizeof(aclspv_wrd2_t) == (8) ? 1 : -1];
ae2f_unused typedef aclspv_wrd_t b32_v[sizeof(aclspv_wrd_t) == (4) ? 1 : -1];

#define	ZERO32	0
#define	ZERO64	0

#if	(_ae2f_gnuc(!)0 || _ae2f_clang(!)0)
typedef aclspv_wrd_t b128 __attribute__((vector_size(sizeof(b32) << 2)));
typedef aclspv_wrd_t b256 __attribute__((vector_size(sizeof(b32) << 3)));
#define	ZERO128	((b128) {0, 0, 0, 0})
#define	ZERO256	((b256) {0, 0, 0, 0, 0, 0, 0, 0})
#else
typedef struct { uint32_t m0, m1, m2, m3; } b128;
ae2f_inline_f static ae2f_ccconst b128 s_z128(void) {
	const b128 Z = { 0, 0, 0, 0 };
	return Z;
}
#define	ZERO128	s_z128()

typedef struct { uint32_t m0, m1, m2, m3, m4, m5, m6, m7; } b256;
ae2f_inline_f static ae2f_ccconst b256 s_z256(void) {
	const b256 Z = { 0, 0, 0, 0, 0, 0, 0, 0 };
	return Z;
}
#define	ZERO256	s_z256()
#endif


ae2f_unused typedef aclspv_wrd_t b128_v[sizeof(b128) == 0x10 ? 1 : -1];
ae2f_unused typedef aclspv_wrd_t b256_v[sizeof(b256) == 0x20 ? 1 : -1];

#endif
