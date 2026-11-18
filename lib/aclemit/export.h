#ifndef	aclemit_export_h
#define	aclemit_export_h

#include <aclspv.h>

/**
 * build phase: decorate the exported functions.
 *
 * a defined function is exported when its declaration says so
 * (`__declspec(dllexport)` / `visibility("default")`), or when
 * ACLSPV_CTX_FLAG0_EXPORT_BY_DEFAULT is set and it is not
 * `visibility("hidden")`.
 *
 * every target must have an OpFunction already (the compile phase or the
 * build phase emitted its body); entry points have no symbol and are left
 * alone. adds the Linkage capability on the first export.
 * */
aclspv_compile_t aclemit_export(h_aclspv_ctx_t h_ctx, CXCursor c_root);

#endif
