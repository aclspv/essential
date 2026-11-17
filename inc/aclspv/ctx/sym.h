#ifndef	aclspv_ctx_sym_h
#define aclspv_ctx_sym_h

#include "./common.h"

#include <aclspv/id.h>
#include <aclspv/link.h>

#include <ae2f/Cxx/extern.h>

/** c should be len of `rd` EXCLUDING null terminator (\0) */
ae2f_extern ACLSPV_ABI_LINK aclspv_id_t aclspv_cfg_symbol(
		const char* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_id_t	c_new_opt,
		h_aclspv_ctx_t h
		);

#endif
