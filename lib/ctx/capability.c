#include <aclspv.h>
#include <ae2f/cc.h>
#include <ae2f/bll.h>
#include <spirv/unified1/spirv.h>
#include <util/emitx.h>

ACLSPV_ABI_IMPL ae2f_ccpure aclspv_bool_t aclspv_chck_capability_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const enum SpvCapability_		c_cap
		) {
	const aclspv_wrd_t	COUNT = rd_ctx 
		? rd_ctx->m_count.m_capability >> (aclspv_wrd_t)1 
		: (aclspv_wrd_t)0;

#define	KEY	((aclspv_wrd_t)c_cap)

	aclspv_wrd_t* const ae2f_restrict BASE	= rd_ctx->m_section.m_capability.m_p;

	aclspv_wrd_t	LR[2];

	ae2f_ifezerr(rd_ctx)		return 0;
	ae2f_ifezerr(c_cap)		return 0;
	ae2f_ifnezerr(rd_ctx->m_err)	return 0;
	ae2f_ifezerr(BASE)		return 0;

#define	LEFT		LR[1]
#define	RIGHT		LR[0]


	LEFT	= 0;
	RIGHT	= COUNT;

	while(LEFT < RIGHT) {
		const aclspv_wrd_t	MID = LEFT + ((RIGHT - LEFT) >> 1);
		if(BASE[(MID << 1) + 1] == KEY)	return	1;
		LR[BASE[(MID << 1) + 1] < KEY] = MID + (BASE[(MID << 1) + 1] < KEY);
	}

	return LEFT < COUNT && BASE[(LEFT << 1) + 1] == KEY;

#undef	LEFT
#undef	RIGHT
#undef	KEY
}

ACLSPV_ABI_IMPL aclspv_bool_t aclspv_add_capability_from_ctx(
		h_aclspv_ctx_t				h_ctx,
		const enum SpvCapability_		c_cap
		) {
	const aclspv_wrd_t	COUNT = h_ctx 
		? (h_ctx->m_count.m_capability >> (aclspv_wrd_t)1)
		: (aclspv_wrd_t)0;

#define	KEY	((aclspv_wrd_t)c_cap)

	aclspv_wrd_t* ae2f_restrict BASE	= h_ctx->m_section.m_capability.m_p;

	aclspv_wrd_t	LR[2];

	ae2f_ifezerr(h_ctx)		return 0;
	ae2f_ifnezerr(h_ctx->m_err)	return 0;
	ae2f_ifezerr(c_cap)		return 0;

#define	LEFT		LR[1]
#define	RIGHT		LR[0]

	LEFT	= 0;
	RIGHT	= COUNT;

	if(BASE) while(LEFT < RIGHT) {
		const aclspv_wrd_t	MID = LEFT + ((RIGHT - LEFT) >> 1);
		if(BASE[(MID << 1) + 1] == KEY)	return	1;
		LR[BASE[(MID << 1) + 1] < KEY] = MID + (BASE[(MID << 1) + 1] < KEY);
	}

	if(LEFT < COUNT && BASE[(LEFT << 1) + 1] == KEY) return 1;

	ae2f_ifezerr(h_ctx->m_count.m_capability = aclutil_emitx_2(
				&h_ctx->m_section.m_capability
				, h_ctx->m_count.m_capability
				, SpvOpCapability
				, KEY)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CAPABILITY;
		return 0;
	}

	BASE	= ((aclspv_wrd_t*)(h_ctx->m_section.m_capability.m_p));

	if(COUNT > LEFT) {
		aclspv_wrd_t		C = COUNT;

		while(C-- > LEFT) {
			BASE[(C << 1) + 3] = BASE[(C << 1) + 1];
		}
	}	/** memmove */


	BASE[(LEFT << 1) + 1] = KEY;
	return 1;

#undef	LEFT
#undef	RIGHT
#undef	KEY
}
