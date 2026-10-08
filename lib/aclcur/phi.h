#ifndef	aclcur_phi_h
#define aclcur_phi_h

#include <aclspv.h>

typedef struct aclcur_phi {
	aclspv_id_t	m_lbl;
	aclspv_id_t	m_val;
	aclspv_id_t	m_nxt;
} aclcur_phi_t;

#include "./flag0.h"

enum ACLCUR_FLAG0_PHI_ {
	/** phi chains created by the value flow (load.c) emit standalone through
	 * the block hoist of emit.c; LOAD-attached chains (aclcur_cfg_load_phi)
	 * keep emitting through their owning load hook. this bit, clear of the
	 * class-local EXPLICIT bits (bit 4), is the discriminator.
	 *
	 * standalone phi nodes follow the universal value-node convention:
	 * m_flag0 carries ACLCUR_FLAG0_VALUE and m_val is the node's own result
	 * id; the operand of each incoming edge lives in m_d.m_phi.m_val. */
	ACLCUR_FLAG0_PHI_SELF = ACLCUR_FLAG0_MAX,

	/** authoritative loop-header phi chain (see load.c): the one-entry
	 * chain created by the header's own read, completed post-carve by the
	 * carry fill. distinguishes loop chains from ordinary join chains. */
	ACLCUR_FLAG0_PHI_AUTH,
	ACLCUR_FLAG0_PHI_MAX
};

aclcur_flag0_tent(ACLCUR_FLAG0_PHI_, aclcur_flag0_phi);

/** first node of the phi chain containing c_phi (the node no other phi
 * node of the same block references through its m_nxt), 0 when c_phi is
 * not a phi node or the chain is broken. */
aclspv_id_t aclcur_phi_head(aclspv_id_t c_phi, h_aclspv_ctx_t h_ctx);

/** emit the whole chain headed by c_head as one OpPhi instruction. */
aclspv_compile_t aclcur_phi_emit(aclspv_id_t c_head, h_aclspv_ctx_t h_ctx);

/** 1 on failure */
aclspv_bool_t aclcur_cfg_load_phi(
		aclspv_id_t c_load,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		);

#endif
