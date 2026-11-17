#ifndef	aclspv_ver_spv_h
#define aclspv_ver_spv_h

#include "./common.h"
#include <aclspv/wrd.h>

/** 
 * represents version c_mjr.c_mnr in 32-bits (aclspv_wrd_t)
 *
 * c_mjr: u8	; majour version
 * c_mnr: u8	; minour version
 * */
#define	aclspv_ver_spv(c_mjr, c_mnr)	\
	(aclspv_and_pad(c_mjr, 0xFF, 16) | aclspv_and_pad(c_mnr, 0xFF, 8))

typedef aclspv_wrd_t	aclspv_ver_spv_t;

#endif
