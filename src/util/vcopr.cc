#ifndef	aclutil_vcopr_cc
#define aclutil_vcopr_cc

#include <aclspv.h>
#include <ae2f/cgoto.h>
#include <ae2f/pat/macrofunc.h>

#if	ae2f_cgoto_available
#define	CASE(L, A)		L##A

#define	LBLDECL(L)		\
	void*	L##LBL[]	= { &&L##0, &&L##1, &&L##2, &&L##3, &&L##4, &&L##5, &&L##6, &&L##7 }

#define	SWITCH(L, A)	ae2f_cgoto_beg(); { LBLDECL(L); goto *L##LBL[A]; } ae2f_cgoto_end();
#define	FALLTHROUGH
#define	DEFAULT

#else
#define	CASE(L, A)		case A
#define	SWITCH(L, A)	switch(A)
#define	FALLTHROUGH	ae2f_fallthrough
#define	DEFAULT		default: ae2f_unreachable(); return
#define	LBLDECL(L)
#endif

#if	!__ae2f_MACRO_GENERATED

#define	k_oper	|=

#endif

/** @brief I'm already screwed, so I'll enjoy it. Let's all die. - DXDXFFG */
ae2f_MAC((L, k_oper, )) aclutil_vcopr(
		aclspv_wrd_t* const ae2f_restrict r,
		const aclspv_wrd_t* const ae2f_restrict rd,
		aclspv_wrd_t c) {

	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic push")));
	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic ignored \"-Wanalyzer-use-of-uninitialized-value\"")));
	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")));

	{
		aclspv_wrd_t	L$$N8 = ((c) + 7) >> 3;
		const aclspv_wrd_t* ae2f_restrict	L$$SRC	= (rd);
		aclspv_wrd_t* ae2f_restrict		L$$DST	= (r);

		SWITCH(L, (c) & 7) {
			DEFAULT;

			do {
				CASE(L, 0): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 7): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 6): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 5): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 4): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 3): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 2): *L$$DST++ k_oper *L$$SRC++; FALLTHROUGH;
				CASE(L, 1): *L$$DST++ k_oper *L$$SRC++;
			} while(--L$$N8 > 0);
		}
	}

	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic pop")));
}

#endif
