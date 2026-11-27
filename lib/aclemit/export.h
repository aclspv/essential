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

#ifndef	aclemit_export_h
#define	aclemit_export_h

#include <aclspv.h>

/**
 * build phase: decorate the exported functions.
 *
 * a defined function is exported when its declaration says so
 * (`__declspec(dllexport)` / `visibility("default")`), or when
 * ACLSPV_CTX_FLAG0_EXPORT_BY_DEFAULT is set and it is not
 * `visibility("hidden")`. the declaration says so through the symbol the
 * gather pass recorded (ACLSYM_FN_FLAG0_DECL_EXPORT / _DECL_HIDDEN), not
 * through a cursor: a build phase that takes no source has none to read.
 *
 * every target must have an OpFunction already (the compile phase or the
 * build phase emitted its body); entry points are left alone. adds the Linkage
 * capability on the first export.
 * */
aclspv_compile_t aclemit_export(h_aclspv_ctx_t h_ctx);

#endif
