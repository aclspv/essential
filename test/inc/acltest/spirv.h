#ifndef	acltest_spirv_h
#define	acltest_spirv_h

#include <aclspv.h>

/** opcodes this checker understands. */
enum ACLTEST_SPV_OP_ {
	ACLTEST_SPV_OP_PHI			= 245,
	ACLTEST_SPV_OP_LOOP_MERGE		= 246,
	ACLTEST_SPV_OP_SELECTION_MERGE		= 247,
	ACLTEST_SPV_OP_LABEL			= 248,
	ACLTEST_SPV_OP_BRANCH			= 249,
	ACLTEST_SPV_OP_BRANCH_CONDITIONAL	= 250
};

typedef struct {
	unsigned	m_num_inst;
	unsigned	m_num_lbl;
	unsigned	m_num_br;
	unsigned	m_num_brc;
	unsigned	m_num_selmerge;
	unsigned	m_num_loopmerge;
	unsigned	m_num_phi;

	/** every merge / branch target names a defined OpLabel. */
	int		m_targets_ok;
} acltest_spirv_stats;

/** true when `c_id` is the result id of an OpLabel. */
static int s_acltest_spirv_has_lbl(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_id
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return 0;

		if(op == ACLTEST_SPV_OP_LABEL && wc >= 2 && rd[i + 1] == c_id)
			return 1;

		i += wc;
	}

	return 0;
}

/**
 * walk the instruction stream and fill `r`.
 *
 * returns
 * :  0	well formed
 * : -1	null argument / too short
 * : -2	bad magic
 * : -3	truncated instruction
 * */
static int acltest_spirv_scan(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		acltest_spirv_stats*	r
		) {
	aclspv_wrdcount_t i;

	if(!rd || !r || c < 5)
		return -1;

	if(rd[0] != (aclspv_wrd_t)0x07230203u)
		return -2;

	r->m_num_inst		= 0;
	r->m_num_lbl		= 0;
	r->m_num_br		= 0;
	r->m_num_brc		= 0;
	r->m_num_selmerge	= 0;
	r->m_num_loopmerge	= 0;
	r->m_num_phi		= 0;
	r->m_targets_ok		= 1;

	for(i = 5; i < c; ) {
		const unsigned		wc = (unsigned)(rd[i] >> 16);
		const unsigned		op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return -3;

		++r->m_num_inst;

		switch(op) {
			case ACLTEST_SPV_OP_LABEL:
				++r->m_num_lbl;
				break;

			case ACLTEST_SPV_OP_PHI:
				++r->m_num_phi;
				break;

			case ACLTEST_SPV_OP_BRANCH:
				++r->m_num_br;
				if(wc < 2 || !s_acltest_spirv_has_lbl(rd, c, rd[i + 1]))
					r->m_targets_ok = 0;
				break;

			case ACLTEST_SPV_OP_BRANCH_CONDITIONAL:
				/** [condition][true][false] */
				++r->m_num_brc;
				if(wc < 4
						|| !s_acltest_spirv_has_lbl(rd, c, rd[i + 2])
						|| !s_acltest_spirv_has_lbl(rd, c, rd[i + 3]))
					r->m_targets_ok = 0;
				break;

			case ACLTEST_SPV_OP_SELECTION_MERGE:
				++r->m_num_selmerge;
				if(wc < 2 || !s_acltest_spirv_has_lbl(rd, c, rd[i + 1]))
					r->m_targets_ok = 0;
				break;

			case ACLTEST_SPV_OP_LOOP_MERGE:
				++r->m_num_loopmerge;
				if(wc < 3
						|| !s_acltest_spirv_has_lbl(rd, c, rd[i + 1])
						|| !s_acltest_spirv_has_lbl(rd, c, rd[i + 2]))
					r->m_targets_ok = 0;
				break;

			default:
				break;
		}

		i += wc;
	}

	return 0;
}

#endif
