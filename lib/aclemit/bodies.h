/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

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
 * (recursion) or a callee that never got a body, reported as
 * ACLSPV_COMPILE_RECURSIVE_CALL. a definition of a function the module already
 * declares as an import is the other contradiction, reported as
 * ACLSPV_COMPILE_IMPORT_DEFINED.
 * */
aclspv_compile_t aclemit_build_bodies(h_aclspv_ctx_t h_ctx, CXCursor c_root);

/**
 * true when a body an earlier compile pass recorded still has no OpFunction:
 * the compile phase deferred it, and completing it means walking the
 * translation unit it came from.
 *
 * asks the symbol cache only, so it is answerable after that translation unit
 * is gone. entry points have no symbol and are not covered — they can only be
 * answered by a walk.
 * */
aclspv_bool_t aclemit_body_pending(h_aclspv_ctx_t h_ctx);

/**
 * true when a body that is still pending cannot be completed by any phase: its
 * waiting is a call cycle, not an import.
 *
 * an internal-linkage function is never imported — its body must be emitted
 * from the translation unit that declares it, and the compile phase already
 * sorts that unit's bodies by dependency. a static body still pending after
 * the compile phase is therefore a cycle (ACLSPV_COMPILE_RECURSIVE_CALL),
 * never a missing source.
 * */
aclspv_bool_t aclemit_body_cyclic(h_aclspv_ctx_t h_ctx);

#endif
