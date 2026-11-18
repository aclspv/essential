#ifndef	aclemit_bodies_h
#define	aclemit_bodies_h

#include <aclspv.h>

/**
 * body scheduling: emit the main-file bodies in dependency order.
 *
 * a body is emitted only when every call in it names something the module
 * already has (aclsym_fn_ready / aclsym_body_ready), so definition order in
 * the source does not matter: a body that calls a function defined further
 * down simply waits for a later pass. the two phases differ in what they do
 * with the leftovers.
 * */

/**
 * COMPILE phase: emit every body that can be emitted now.
 *
 * bodies that call a function with no body in this translation unit stay
 * pending — their import is the build phase's job. bodies in a call cycle
 * stay pending too; the build phase reports that.
 *
 * c_root belongs to a translation unit that is alive: nothing here stores a
 * cursor beyond this call.
 * */
aclspv_compile_t aclemit_compile_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root);

/**
 * BUILD phase: complete what the compile phase deferred.
 *
 * collects the bodies with no OpFunction yet, declares the imports they need
 * (prepended: SPIR-V requires declarations before definitions), then runs the
 * same readiness queue. a body that never becomes ready is a call cycle
 * (recursion) or a callee that never got a body — reported as
 * ACLSPV_COMPILE_RECURSIVE_CALL.
 * */
aclspv_compile_t aclemit_build_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root);

#endif
