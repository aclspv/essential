#ifndef	aclemit_count_fn_h
#define	aclemit_count_fn_h

/** @file count_fn.h */

#include <ae2f/Keys.h>

#include <aclspv3rd/clang/idx.h>

ae2f_extern enum CXChildVisitResult aclemit_iter_entry_point(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		);

ae2f_extern enum CXChildVisitResult aclemit_count_fn(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		);


#endif
