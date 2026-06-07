#ifndef	aclutil_vcopr_cc
#define aclutil_vcopr_cc

#include <aclspv.h>
#include <ae2f/cgoto.h>
#include <ae2f/pat/macrofunc.h>

#if	ae2f_cgoto_available
#define	aclutil_vcopr_case(L, A)		L##A

#define	aclutil_vcopr_lbldcl(L)		\
	void*	L##LBL[]	= { &&L##0, &&L##1, &&L##2, &&L##3, &&L##4, &&L##5, &&L##6, &&L##7 }

#define	aclutil_vcopr_swtch(L, A)	ae2f_cgoto_beg(); { aclutil_vcopr_lbldcl(L); goto *L##LBL[A]; } ae2f_cgoto_end();
#define	aclutil_vcopr_fall
#define	aclutil_vcopr_default

#else
#define	aclutil_vcopr_case(L, A)		case A
#define	aclutil_vcopr_swtch(L, A)	switch(A)
#define	aclutil_vcopr_fall	ae2f_fallthrough
#define	aclutil_vcopr_default		default: ae2f_unreachable(); return
#define	aclutil_vcopr_lbldcl(L)
#endif

#if	!__ae2f_MACRO_GENERATED

#define	k_oper		|=
#define	k_oper_inc	++

#endif

/** @brief I'm already screwed, so I'll enjoy it. Let's all die. - DXDXFFG */
ae2f_MAC((L, k_oper, k_oper_inc, )) aclutil_vcopr_v(
		aclspv_wrd_t* const ae2f_restrict r,
		const aclspv_wrd_t* const ae2f_restrict rd,
		const aclspv_wrd_t c) {
	;
	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic push")));
	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic ignored \"-Wanalyzer-use-of-uninitialized-value\"")));
	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")));

	{
		aclspv_wrd_t	L$$N8 = ((c) + 7) >> 3;
		const aclspv_wrd_t* ae2f_restrict	L$$SRC	= (rd);
		aclspv_wrd_t* ae2f_restrict		L$$DST	= (r);

		aclutil_vcopr_swtch(L, (c) & 7) {
			aclutil_vcopr_default;

			do {
				aclutil_vcopr_case(L, 0): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 7): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 6): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 5): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 4): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 3): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 2): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc; aclutil_vcopr_fall;
				aclutil_vcopr_case(L, 1): *L$$DST k_oper_inc k_oper *L$$SRC k_oper_inc;
			} while(--L$$N8 > 0);
		}
	}

	;
	_ae2f_gnuc(N_ae2f_clang(_Pragma("GCC diagnostic pop")));
}

#if !__ae2f_MACRO_GENERATED
#define	_aclutil_vcopr_v(L, k_oper, k_oper_inc, r, rd, c) aclutil_vcopr_v(r, rd, c)
#endif

ae2f_MAC((L, k_oper, ))	aclutil_vcopr(
		aclspv_wrd_t* const ae2f_restrict r,
		const aclspv_wrd_t* const ae2f_restrict rd,
		const aclspv_wrd_t c) {
	_aclutil_vcopr_v(L, k_oper, ++, r, rd, c);
}

#endif
