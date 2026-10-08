#ifndef	aclcur_j_h
#define aclcur_j_h

#include <aclspv.h>

#include "./j/emit.h"

typedef struct {
	aclspv_wrd_t	m_lbl;
} aclcur_j_t;

aclspv_id_t	aclcur_cfg_jmp(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx);

#endif
