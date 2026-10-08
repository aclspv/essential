#ifndef	aclcfg_ptr_h
#define aclcfg_ptr_h

#include <aclspv.h>

ae2f_ccpure static aclspv_wrd_t s_aclcfg_get_ptrsz(h_aclspv_ctx_t h) {
	switch(h->m_usrcfg.m_mdl_addr) {
		default:
			return 0;
		case SpvAddressingModelPhysical32:
			return 4;
		case SpvAddressingModelPhysical64:	
		case SpvAddressingModelPhysicalStorageBuffer64:
			return 8;
	}

	ae2f_unreachable();
}

#endif
