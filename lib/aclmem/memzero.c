#include "../aclmem.h"

#include <ae2f/cc/profiles.h>
#include <.aclspv/mem/zero.h>


#if	(_ae2f_gnuc(!)0 || _ae2f_clang(!)0)
typedef aclspv_wrd_t b128 __attribute__((vector_size(sizeof(uint32_t) << 2)));
#define	ZERO	((b128) {0, 0, 0, 0})
#else
ae2f_inline_f typedef struct { uint32_t m0, m1; } b128;
static ae2f_ccconst b128 zero(void) {
	const b128 Z = { 0, 0, 0, 0 };
	return Z;
}
#define	ZERO	zero()
#endif

#if	(_ae2f_gnuc(!)0 || _ae2f_clang(!)0)
static ae2f_inline_f void aclmemzero128(void* ae2f_restrict wr, aclspv_wrd_t c) {
	ae2f_unused typedef aclspv_wrd_t b128_v[sizeof(b128) == 16 ? 1 : -1];
	b128* ae2f_restrict p = wr;
	_aclmemzero_duff8(L, aclspv_wrd_t, c, p, ZERO);
}
#else
static ae2f_inline_f void aclmemzero128(void* ae2f_restrict wr, aclspv_wrd_t c) {
	typedef aclspv_wrd_t b128 __attribute__((vector_size(sizeof(uint32_t) << 2)));
	ae2f_unused typedef aclspv_wrd_t b128_v[sizeof(b128) == 16 ? 1 : -1];
	b128* ae2f_restrict p = wr;
	_aclmemzero_duff8(L, aclspv_wrd_t, c, p, ((b128) {0, 0, 0, 0}));
}
#endif

typedef aclspv_wrd2_t b64;
typedef aclspv_wrd_t b32;

ae2f_unused typedef aclspv_wrd2_t b64_v[sizeof(aclspv_wrd2_t) == (8) ? 1 : -1];
ae2f_unused typedef aclspv_wrd_t b32_v[sizeof(aclspv_wrd_t) == (4) ? 1 : -1];

#define	s_case_byte_noskip(c, ptr) case (c): *--(ptr) = 0;
#define	s_case_byte(c, ptr) case (c): *--(ptr) = 0; ae2f_fallthrough
#define	s_case_bulk(c_bit, ptr)	case ((c_bit) >> 3): *((b##c_bit*)(void*)((ptr) -= (c_bit) >> 3)) = 0; break
#define	s_case_skip(c, c_bit, ptr, lbl)	ae2f_ifnezerr_strict(0)  { ae2f_unreachable(); case c: *((b##c_bit*)(void*)((ptr) -= (c_bit) >> 3)) = 0; } goto lbl

void aclmemzero(void* ae2f_restrict wr, aclspv_wrd_t c) {
	char* ae2f_restrict		wr_end = (char*)wr + c;

	{
		aclspv_wrd_t rem128 = aclmemremaddr(wr_end, 16);

		if(c > rem128) {
			c -= rem128;
#define	BULK_LBL(c)	BULK##c
			switch(rem128) {
				s_case_byte(0xF, wr_end);
				s_case_byte(0xE, wr_end);
				s_case_byte_noskip(0xD, wr_end);
				s_case_skip(0xC, 32, wr_end, BULK64);

				s_case_byte(0xB, wr_end);
				s_case_byte(0xA, wr_end);
				s_case_byte_noskip(0x9, wr_end);

				BULK64:
				s_case_bulk(64, wr_end);

				s_case_byte(0x7, wr_end);
				s_case_byte(0x6, wr_end);
				s_case_byte(0x5, wr_end);
				s_case_bulk(32, wr_end);

				s_case_byte(0x3, wr_end);
				s_case_byte(0x2, wr_end);
				s_case_byte(0x1, wr_end);

				case 0x0:
				break;

				default:
				ae2f_unreachable();
			}
		}
	}

	{
		const aclspv_wrd_t c128 = c & ~((aclspv_wrd_t)0xF);
		aclmemzero128((wr_end -= (c128)), c128 >> 4);
		c -= c128;
	}

	/** wr_end is aligned as 128 bit here guaranteed. */

#undef	BULK_LBL
#define	BULK_LBL(c)	BULK2##c
	switch(c) {
		s_case_byte(0xF, wr_end);
		s_case_byte(0xE, wr_end);
		s_case_byte_noskip(0xD, wr_end);
		s_case_skip(0xC, 32, wr_end, BULK64_2);

		s_case_byte(0xB, wr_end);
		s_case_byte(0xA, wr_end);
		s_case_byte_noskip(0x9, wr_end);

BULK64_2:
		s_case_bulk(64, wr_end);

		s_case_byte(0x7, wr_end);
		s_case_byte(0x6, wr_end);
		s_case_byte(0x5, wr_end);
		s_case_bulk(32, wr_end);

		s_case_byte(0x3, wr_end);
		s_case_byte(0x2, wr_end);
		s_case_byte(0x1, wr_end);

		case 0x0:
		break;

		default:
		ae2f_unreachable();
	}
}

#undef	s_case_byte
#undef	s_case_bulk

