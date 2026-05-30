#include <aclspv.h>
#include <stdlib.h>
#include <util/wrdemit.h>

#include <.aclspv/util/vcopr.h>

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_dump_ctx_output1(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const	rdwr_count_ret,
		aclspv_wrd_t* ae2f_restrict const	ret
		) {	
	aclspv_wrd_t			NUM_RET;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifezerr(rdwr_count_ret)	return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	NUM_RET	
		= h_ctx->m_count.m_capability
		+ h_ctx->m_count.m_ext
		+ h_ctx->m_count.m_memmodel
		+ h_ctx->m_count.m_entp
		+ h_ctx->m_count.m_execmode
		+ h_ctx->m_count.m_name
		+ h_ctx->m_count.m_decorate
		+ h_ctx->m_count.m_types
		+ h_ctx->m_count.m_vars
		+ h_ctx->m_count.m_fndef
		+ 5;

	if(*rdwr_count_ret < NUM_RET)	{ 
		*rdwr_count_ret = NUM_RET;
		return ACLSPV_COMPILE_INSUFFICIENT_BUFFER_LENGTH; 
	}

	ae2f_ifezerr(ret)		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	ret[0]	=	SpvMagicNumber;
	ret[1]	=	h_ctx->m_spirv_ver;
	ret[2]	=	0;
	ret[3]	=	NUM_RET;
	ret[4]	=	0;

#define copybinary(k_key, c_buf)	if(h_ctx->m_count.k_key)	\
	_aclutil_vcopr(k_key, =, c_buf, h_ctx->m_section.k_key.m_p, h_ctx->m_count.k_key)

	copybinary(m_capability,	ret + 5);

	copybinary(m_ext,		ret + 5 
			+ h_ctx->m_count.m_capability
		  );

	copybinary(m_memmodel,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
		  );

	copybinary(m_entp,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
		  );

	copybinary(m_execmode,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
		  );

	copybinary(m_name,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
		  );

	copybinary(m_decorate,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
		  );

	copybinary(m_types,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
			+ h_ctx->m_count.m_decorate
		  );

	copybinary(m_vars,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
			+ h_ctx->m_count.m_decorate
			+ h_ctx->m_count.m_types
		  );

	copybinary(m_fndef,		ret + 5 
			+ h_ctx->m_count.m_capability 
			+ h_ctx->m_count.m_ext
			+ h_ctx->m_count.m_memmodel
			+ h_ctx->m_count.m_entp
			+ h_ctx->m_count.m_execmode
			+ h_ctx->m_count.m_name
			+ h_ctx->m_count.m_decorate
			+ h_ctx->m_count.m_types
			+ h_ctx->m_count.m_vars
		  );

#undef	copybinary

	return ACLSPV_COMPILE_OK;
}

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_dump_ctx_output(
		const x_aclspv_ctx* ae2f_restrict		h_ctx,
		aclspv_wrd_t* ae2f_restrict const			r_count,
		aclspv_wrd_t* ae2f_restrict* ae2f_restrict const	ret
		) {	
	aclspv_wrd_t			NUM_RET;
	aclspv_wrd_t* ae2f_restrict	RET;

	if(r_count)		*r_count = 0;

	ae2f_ifezerr(h_ctx)		return ACLSPV_COMPILE_PRM_INVAL_NIL;
	ae2f_ifnezerr(h_ctx->m_err)	return h_ctx->m_err;

	NUM_RET	
		= h_ctx->m_count.m_capability
		+ h_ctx->m_count.m_ext
		+ h_ctx->m_count.m_memmodel
		+ h_ctx->m_count.m_entp
		+ h_ctx->m_count.m_execmode
		+ h_ctx->m_count.m_name
		+ h_ctx->m_count.m_decorate
		+ h_ctx->m_count.m_types
		+ h_ctx->m_count.m_vars
		+ h_ctx->m_count.m_fndef
		+ 5;

	if(r_count)		*r_count = NUM_RET;
	unless(ret)		return ACLSPV_COMPILE_OK;

	*ret = RET = malloc(aclutil_count_to_sz(NUM_RET));
	ae2f_ifezerr(RET)	return ACLSPV_COMPILE_ALLOC_FAILED_ASM;

	return aclspv_dump_ctx_output1(h_ctx, &NUM_RET, RET);
}
