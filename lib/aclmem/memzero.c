#include "./util.h"

#include <ae2f/cc/profiles.h>
#include <.aclspv/mem/zero.h>

static ae2f_inline_f void s_mz256(void* ae2f_restrict wr, aclspv_wrd_t c) {
	b256* ae2f_restrict p = wr;
	_aclmemzero_duff8(L, aclspv_wrd_t, c, p, ZERO256);
}

#define	s_case_bytej(c, ptr, lbl) case (c): *--(ptr) = 0; goto lbl
#define	s_case_byte(c, ptr) case (c): *--(ptr) = 0; ae2f_fallthrough
#define	s_case_bulk(c_bit, ptr)	case ((c_bit) >> 3): *((b##c_bit*)(void*)((ptr) -= (c_bit) >> 3)) = ZERO##c_bit; break
#define	s_case_skip(c, c_bit, ptr, lbl)	\
	case c:	\
		*((b##c_bit*)(void*)((ptr) -= (c_bit) >> 3)) = ZERO##c_bit;	\
		goto lbl

void aclmemzero(void* ae2f_restrict wr, aclspv_wrd_t c) {
	char* ae2f_restrict		wr_end = (char*)wr + c;
	aclspv_wrd_t rem256 = aclmemremaddr(wr_end, 32);

	if(c < rem256) {
		while(c --> 0) *--wr_end = 0;
		return;
	}

	c -= rem256;
	switch(rem256) {
		s_case_byte(0x1F, wr_end);
		s_case_byte(0x1E, wr_end);
		s_case_byte(0x1D, wr_end);
		s_case_skip(0x1C, 32, wr_end, L192);
		s_case_byte(0x1B, wr_end);
		s_case_byte(0x1A, wr_end);
		s_case_bytej(0x19, wr_end, L192);
L192:		s_case_skip(0x18, 64, wr_end, L128);
		s_case_byte(0x17, wr_end);
		s_case_byte(0x16, wr_end);
		s_case_byte(0x15, wr_end);
		s_case_skip(0x14, 32, wr_end, L128);
		s_case_byte(0x13, wr_end);
		s_case_byte(0x12, wr_end);
		s_case_bytej(0x11, wr_end, L128);
L128:		s_case_bulk(128, wr_end);
		s_case_byte(0x0F, wr_end);
		s_case_byte(0x0E, wr_end);
		s_case_byte(0x0D, wr_end);
		s_case_skip(0x0C, 32, wr_end, L64);
		s_case_byte(0x0B, wr_end);
		s_case_byte(0x0A, wr_end);
		s_case_bytej(0x09, wr_end, L64);
L64:		s_case_bulk(64, wr_end);
		s_case_byte(0x07, wr_end);
		s_case_byte(0x06, wr_end);
		s_case_byte(0x05, wr_end);
		s_case_bulk(32, wr_end);
		s_case_byte(0x03, wr_end);
		s_case_byte(0x02, wr_end);
		s_case_byte(0x01, wr_end);

		case 0x0:
		break;

		default:
		ae2f_unreachable();
	}

	/** wr_end is aligned as 256 bit here guaranteed. */

	{
		const aclspv_wrd_t c256 = c & ~((aclspv_wrd_t)0x1F);
		s_mz256((wr_end -= (c256)), c256 >> 5);
		c -= c256;
	}

	while(c --> 0) *--wr_end = 0;
}

#undef	s_case_byte
#undef	s_case_bulk

