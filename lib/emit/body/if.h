#ifndef aclemit_body_if_h
#define aclemit_body_if_h

#include <util/fnctx.h>
#include <clang-c/Index.h>

/** @brief it produces error on ctx. */
void aclemit_body_if(CXCursor c_cur, aclutil_fnctx* ae2f_restrict h_fnctx);

#endif
