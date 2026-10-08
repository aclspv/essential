#ifndef	aclcur_load_get_h
#define aclcur_load_get_h

#include "../common.h"

/** 
 * find the load id from function implementation and constant area.
 *
 * returns.m_id		would be non-zero only if it is found from runtime.
 * returns.m_val 	would be non-zero if c_load has been found somewhere.
 * found none, you've got nothing.
 * */
ae2f_ccpure aclcur_t aclcur_get_load(aclspv_id_t c_load, h_aclspv_ctx_t h_ctx);

#endif
