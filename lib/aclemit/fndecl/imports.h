#ifndef	aclemit_fndecl_imports_h
#define	aclemit_fndecl_imports_h

#include <aclspv.h>

/**
 * declare the functions c_body calls that have no body in this translation
 * unit.
 *
 * a body-less normal function is an import: it is referenced by id and
 * resolved at link time, so the module carries an OpFunction declaration
 * (no block) plus
 *
 *   OpDecorate %id LinkageAttributes "<name>" Import
 *
 * and the Linkage capability.
 *
 * the declaration words go to h_wrds / *wr_count, NOT straight into the
 * module: SPIR-V requires every function declaration to precede every
 * function definition, and definitions may already be emitted by the build
 * phase's start. the caller prepends them. the decorations go straight to
 * the decorate section (order there does not matter).
 * */
aclspv_compile_t aclemit_fndecl_imports(
		CXCursor		c_body,
		x_aclspv_vec*		h_wrds,
		aclspv_wrdcount_t* ae2f_restrict	wr_count,
		h_aclspv_ctx_t		h_ctx
		);

#endif
