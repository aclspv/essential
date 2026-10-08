#ifndef	aclty_layout_h
#define aclty_layout_h

#include <aclspv.h>

typedef aclspv_layout_t aclty_layout_t;

#include "./struct.h"

typedef struct {
	aclty_add_struct_mbr_t* m_add_struct_mbr;
	aclspv_wrd_t		m_array_stride;
} aclty_layout_interface_t;

typedef int aclty_layout_match[sizeof(enum ACLSPV_LAYOUT_) == sizeof(aclty_layout_t)];

ae2f_unused ae2f_ccconst static aclty_layout_interface_t aclty_cfg_layout_interface(enum ACLSPV_LAYOUT_ c) {
	aclty_layout_interface_t r;

	switch(c) {
		case ACLSPV_LAYOUT_MAX:
		default:
			r.m_add_struct_mbr = 0;
			r.m_array_stride = 0;
			break;

		case ACLSPV_LAYOUT_STD140:
			r.m_add_struct_mbr	= aclty_add_struct_mbr_std140;
			r.m_array_stride	= 16;
			break;

		case ACLSPV_LAYOUT_STD430:
			r.m_add_struct_mbr	= aclty_add_struct_mbr_std430;
			r.m_array_stride	= 4;
			break;
	}

	return r;
}

#endif
