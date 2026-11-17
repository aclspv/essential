#ifndef	aclspv_ver_vk_h
#define	aclspv_ver_vk_h

#include "./common.h"
#include <aclspv/wrd.h>

/**
 * c_var: u3(u8)	; variant identifier (0 as std)
 * c_mjr: u7(u8)	; majour version number
 * c_mnr: u10(u16)	; minour version number
 * c_patch: u12(u16)	; patch version number
 * */
#define aclspv_ver_vk(c_var, c_mjr, c_mnr, c_patch)	(	\
		aclspv_and_pad(c_patch, 0xFFF, 0) | aclspv_and_pad(c_mnr, 0x3FF, 12) | \
		aclspv_and_pad(c_mjr, 0x7F, 22)	| aclspv_and_pad(c_var, 0x7, 29)	\
		)

#define aclspv_ver_vk2(c_mjr, c_mnr)	\
	aclspv_ver_vk(0x0, c_mjr, c_mnr, 0x0)

typedef aclspv_wrd_t	aclspv_ver_vk_t;

#endif
