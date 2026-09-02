#ifndef	aclmem_copy_cc
#define aclmem_copy_cc

#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/macrofunc.h>

#if	!__ae2f_MACRO_GENERATED
typedef unsigned	idx_t;
typedef char		el_t;
#endif

ae2f_MAC((L, idx_t, )) aclmemcpy_duff8(idx_t& cr_num, el_t*& wr, const el_t*& rd) {
	idx_t L$$IDX = ((cr_num) + 0x7) >> 3;

	if((cr_num)) switch((cr_num) & 0x7) {
		case 0x0:
			do {
				*(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x7: *(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x6: *(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x5: *(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x4: *(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x3: *(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x2: *(wr)++ = *(rd)++; ae2f_fallthrough;
				case 0x1: *(wr)++ = *(rd)++;
			} while(--L$$IDX);
			break;

		default: ae2f_unreachable();
	}
}

#endif
