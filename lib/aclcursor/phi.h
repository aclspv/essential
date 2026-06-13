#ifndef	aclcursor_phi_h
#define	aclcursor_phi_h

#include <aclspv.h>

typedef struct {
	aclspv_wrd_t	m_idx_decl;
	aclspv_wrd_t	m_nxt_phi;
	aclspv_wrd_t	m_prv_phi;
	aclspv_id_t	m_parent;
	aclspv_id_t	m_idval;
} aclcursor_phi_t;

#endif
