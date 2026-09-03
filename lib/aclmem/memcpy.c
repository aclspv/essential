#include "./util.h"
#include <.aclspv/mem/copy.h>


#define s_mcpX_body(x)	\
	b##x* ae2f_restrict w = wr;	\
	const b##x* ae2f_restrict r = rd;	\
	_aclmemcpy_duff8(L, aclspv_wrd_t, c, w, r) \

#define	s_mcpX(x)	\
	static ae2f_inline_f void s_mcp##x(	\
			void* ae2f_restrict wr,	\
			const void* ae2f_restrict rd,	\
			aclspv_wrd_t c	\
			)

typedef char b8;

ae2f_unused s_mcpX(256) { s_mcpX_body(256); }
ae2f_unused s_mcpX(128) { s_mcpX_body(128); }
ae2f_unused s_mcpX(64) { s_mcpX_body(64); }
ae2f_unused s_mcpX(32) { s_mcpX_body(32); }
ae2f_unused s_mcpX(8) { s_mcpX_body(8); }

#define	s_case_bytej(c, r, rd, lbl) case (c): *--(r) = *--(rd); goto lbl
#define	s_case_byte(c, r, rd) case (c): *--(r) = *--(rd); ae2f_fallthrough
#define	s_case_bulk(c_bit, r, rd)	\
	case ((c_bit) >> 3):	\
*((b##c_bit*)(void*)((r) -= (c_bit) >> 3)) = *((const b##c_bit*)(const void*)((rd) -= (c_bit) >> 3));	\
break
#define	s_case_skip(c, c_bit, r, rd, lbl)	\
	case c:	\
*((b##c_bit*)(void*)((r) -= (c_bit) >> 3)) = *((b##c_bit*)(void*)((rd) -= (c_bit) >> 3));	\
goto lbl

typedef void fn_mcpx_t(
		void* ae2f_restrict wr,
		const void* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_wrd_t c_rem);

static fn_mcpx_t	s_mcpx_256,
			s_mcpx_128,
			s_mcpx_64,
			s_mcpx_32;

static void s_mcpx_256(
		void* ae2f_restrict wr,
		const void* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_wrd_t c_rem
		) {
	char* ae2f_restrict		wr_end = c + ((char*)wr);
	const char* ae2f_restrict	rd_end = c + ((const char*)rd);

	if(c_rem > c) {
		s_mcpx_128(wr, rd, c, c_rem & 0xF);
		return;
	}

	c -= c_rem;
	switch(c_rem) {
		s_case_byte(0x1F, wr_end, rd_end);
		s_case_byte(0x1E, wr_end, rd_end);
		s_case_byte(0x1D, wr_end, rd_end);
		s_case_skip(0x1C, 32, wr_end, rd_end, L0192);
		s_case_byte(0x1B, wr_end, rd_end);
		s_case_byte(0x1A, wr_end, rd_end);
		s_case_bytej(0x19, wr_end, rd_end, L0192);
L0192:		s_case_skip(0x18, 64, wr_end, rd_end, L0128);
		s_case_byte(0x17, wr_end, rd_end);
		s_case_byte(0x16, wr_end, rd_end);
		s_case_byte(0x15, wr_end, rd_end);
		s_case_skip(0x14, 32, wr_end, rd_end, L0128);
		s_case_byte(0x13, wr_end, rd_end);
		s_case_byte(0x12, wr_end, rd_end);
		s_case_bytej(0x11, wr_end, rd_end, L0128);
L0128:		s_case_bulk(128, wr_end, rd_end);
		s_case_byte(0xF, wr_end, rd_end);
		s_case_byte(0xE, wr_end, rd_end);
		s_case_byte(0xD, wr_end, rd_end);
		s_case_skip(0xC, 32, wr_end, rd_end, L064);
		s_case_byte(0xB, wr_end, rd_end);
		s_case_byte(0xA, wr_end, rd_end);
		s_case_bytej(0x9, wr_end, rd_end, L064);
L064:		s_case_bulk(64, wr_end, rd_end);
		s_case_byte(0x7, wr_end, rd_end);
		s_case_byte(0x6, wr_end, rd_end);
		s_case_byte(0x5, wr_end, rd_end);
		s_case_bulk(32, wr_end, rd_end);
		s_case_byte(0x3, wr_end, rd_end);
		s_case_byte(0x2, wr_end, rd_end);
		s_case_byte(0x1, wr_end, rd_end);

		case 0x0:	break;
		default:	ae2f_unreachable();
	}

	{
		const aclspv_wrd_t c256 = c & ~((aclspv_wrd_t)0x1F);
		s_mcp256((wr_end -= (c256)), rd_end -= c256, c256 >> 5);
		c -= c256;
	}

	while(c --> 0) *--wr_end = *--rd_end;
}

static void s_mcpx_128(
		void* ae2f_restrict wr,
		const void* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_wrd_t c_rem
		) {
	char* ae2f_restrict		wr_end = c + ((char*)wr);
	const char* ae2f_restrict	rd_end = c + ((const char*)rd);

	if(c_rem > c) {
		s_mcpx_64(wr, rd, c, c_rem & 0x7);
		return;
	}

	c -= c_rem;
	switch(c_rem) {
		s_case_byte(0xF, wr_end, rd_end);
		s_case_byte(0xE, wr_end, rd_end);
		s_case_byte(0xD, wr_end, rd_end);
		s_case_skip(0xC, 32, wr_end, rd_end, L064);
		s_case_byte(0xB, wr_end, rd_end);
		s_case_byte(0xA, wr_end, rd_end);
		s_case_bytej(0x9, wr_end, rd_end, L064);
L064:		s_case_bulk(64, wr_end, rd_end);
		s_case_byte(0x7, wr_end, rd_end);
		s_case_byte(0x6, wr_end, rd_end);
		s_case_byte(0x5, wr_end, rd_end);
		s_case_bulk(32, wr_end, rd_end);
		s_case_byte(0x3, wr_end, rd_end);
		s_case_byte(0x2, wr_end, rd_end);
		s_case_byte(0x1, wr_end, rd_end);

		case 0x0:	break;
		default:	ae2f_unreachable();
	}

	{
		const aclspv_wrd_t c128 = c & ~((aclspv_wrd_t)0xF);
		s_mcp128((wr_end -= (c128)), rd_end -= c128, c128 >> 4);
		c -= c128;
	}

	while(c --> 0) *--wr_end = *--rd_end;
}

static void s_mcpx_64(
		void* ae2f_restrict wr,
		const void* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_wrd_t c_rem
		) {
	char* ae2f_restrict		wr_end = c + ((char*)wr);
	const char* ae2f_restrict	rd_end = c + ((const char*)rd);

	if(c_rem > c) {
		s_mcpx_32(wr, rd, c, c_rem & 0x3);
		return;
	}

	c -= c_rem;
	switch(c_rem) {
		s_case_byte(0x7, wr_end, rd_end);
		s_case_byte(0x6, wr_end, rd_end);
		s_case_byte(0x5, wr_end, rd_end);
		s_case_bulk(32, wr_end, rd_end);
		s_case_byte(0x3, wr_end, rd_end);
		s_case_byte(0x2, wr_end, rd_end);
		s_case_byte(0x1, wr_end, rd_end);

		case 0x0:	break;
		default:	ae2f_unreachable();
	}

	{
		const aclspv_wrd_t c64 = c & ~((aclspv_wrd_t)0x7);
		s_mcp64((wr_end -= (c64)), rd_end -= c64, c64 >> 3);
		c -= c64;
	}

	while(c --> 0) *--wr_end = *--rd_end;
}

static void s_mcpx_32(
		void* ae2f_restrict wr,
		const void* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_wrd_t c_rem
		) {
	char* ae2f_restrict		wr_end = c + ((char*)wr);
	const char* ae2f_restrict	rd_end = c + ((const char*)rd);

	if(c < c_rem) {
		s_mcp8(wr, rd, c);
		return;
	}

	c -= c_rem;
	switch(c_rem) {
		s_case_byte(0x3, wr_end, rd_end);
		s_case_byte(0x2, wr_end, rd_end);
		s_case_byte(0x1, wr_end, rd_end);

		case 0x0:	break;
		default:	ae2f_unreachable();
	}

	{
		const aclspv_wrd_t c32 = c & ~((aclspv_wrd_t)0x3);
		s_mcp32((wr_end -= (c32)), rd_end -= c32, c32 >> 2);
		c -= c32;
	}

	while(c --> 0) *--wr_end = *--rd_end;
}

void	aclmemcpy(void* ae2f_restrict wr, const void* ae2f_restrict rd, aclspv_wrd_t c) {
#if	1
	char* ae2f_restrict		wr_end = c + ((char*)wr);
	const char* ae2f_restrict	rd_end = c + ((const char*)rd);

	const aclspv_wrd_t
		rem256_w = aclmemremaddr(wr_end, 0x20)
		, rem256_r = aclmemremaddr(rd_end, 0x20);

	aclspv_wrd_t rem256_d = rem256_r < rem256_w ? rem256_w - rem256_r : rem256_r - rem256_w;

	if(rem256_d > c) {
		s_mcp8(wr, rd, c);
		return;
	}

	switch(rem256_d) {
		default:
			ae2f_unreachable();

		case 0x0:
			s_mcpx_256(wr, rd, c, rem256_w);
			break;

		case 0x10:
			s_mcpx_128(wr, rd, c, rem256_w & 0xF);
			return;

		case 0x18:
		case 0x8:
			s_mcpx_64(wr, rd, c, rem256_w & 0x7);
			break;

		case 0x1C:
		case 0x14:
		case 0xC:
		case 0x4:
			s_mcpx_32(wr, rd, c, rem256_w & 0x3);
			break;


		case 0x1F:
		case 0x1E:
		case 0x1D:
		case 0x1B:
		case 0x1A:
		case 0x19:
		case 0x17:
		case 0x16:
		case 0x15:
		case 0x13:
		case 0x12:
		case 0x11:
		case 0xF:
		case 0xE:
		case 0xD:
		case 0xA:
		case 0x9:
		case 0x7:
		case 0x6:
		case 0x5:
		case 0x3:
		case 0x2:
		case 0x1:
			s_mcp8(wr, rd, c);
			return;
	}
#else
	s_mcp8(wr, rd, c);
#endif
}
