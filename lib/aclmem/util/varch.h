#ifndef	aclmem_varch_h
#define aclmem_varch_h

#include <aclspv.h>
#include <ae2f/cc/profiles.h>

/** @file varch.h
 * Marks, per host architecture, whether a fixed-width 128-bit (b128) and
 * 256-bit (b256) vector of 32-bit lanes is available, then defines them.
 *
 * b128 and b256 are HOST-side helpers: aclmemzero and aclmemcpy operate on
 * compiler-internal memory, so the macros below describe the ISA of the
 * machine running aclspv, not the SPIR-V target. The choice is fixed when
 * this header is compiled; a host with AVX2 still gets the aggregate unless
 * it was built with AVX2 enabled.
 *
 * Where no fixed-width vector exists, a plain-C aggregate is used instead.
 * sizeof matches the vector on every target, so the fallback is always
 * correct, only slower. _Alignof does NOT match, so these types must stay
 * out of structs, unions and arrays.
 *
 * Both macros are defined on every path, as -Wundef is enabled.
 * */

/* 128-bit: 4 x 32-bit lanes. Any arch with a 128-bit SIMD register.
 * __ARM_NEON is the only spelling defined on BOTH arm32 and aarch64;
 * aarch64 does not define __ARM_NEON__. __VEC__ is s390x (-mzvector).
 * __loongarch_simd is gcc-only, hence __loongarch_sx. */
#if	defined(__SSE__) || defined(__SSE2__) \
	|| defined(__ARM_NEON) || defined(__ARM_NEON__) \
	|| defined(__wasm_simd128__) \
	|| defined(__ALTIVEC__) || defined(__VSX__) || defined(__POWER8_VECTOR__) \
	|| defined(__mips_msa) \
	|| defined(__loongarch_sx) \
	|| defined(__VEC__) \
	|| defined(__riscv_vector) || defined(__ARM_FEATURE_SVE)
#define	b128_avail	1
#else
#define	b128_avail	0
#endif

/* 256-bit: 8 x 32-bit lanes. Only x86 with AVX, and loongarch64 with LASX,
 * have a fixed 256-bit vector. arm SVE and riscv V are scalable with no
 * compile-time width guarantee, and s390x z14 splits into 2x16B, so all of
 * them take the aggregate below.
 *
 * __AVX2__ rather than __AVX__: avx1 has no 256-bit integer alu, so a
 * __AVX__ gate would lower correctly but badly for any arithmetic, and
 * would drop the minimum host from haswell to sandy bridge. aclmem only
 * ever stores whole vectors, so __AVX__ would be sound in practice; the
 * stricter gate is kept as the conservative choice. */
#if	defined(__AVX2__) || defined(__loongarch_asx)
#define	b256_avail	1
#else
#define	b256_avail	0
#endif

#endif
