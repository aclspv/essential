#ifndef	aclcur_j_h
#define aclcur_j_h

#include <aclcur.h>

#include "./j/emit.h"
#include "./lbl.h"

typedef struct {
	aclspv_wrd_t	m_lbl;
} aclcur_j_t;

typedef aclcur_id_t aclcur_jid_t;

aclcur_jid_t	aclcur_cfg_jmp(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx);

#endif
