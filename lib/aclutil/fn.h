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

#ifndef	aclutil_fn_h
#define	aclutil_fn_h

#include <aclspv.h>
#include <clang-c/Index.h>
#include <string.h>

#include <aclsym/fn.h>
#include <aclattr/fnctrl.h>

/** a C++ method, constructor, or destructor: the declaration kinds that can
 * carry an implicit `this`. */
ae2f_inline_f static aclspv_bool_t aclutil_fn_is_method(CXCursor c_fn) {
	switch((aclspv_cxcurkind_t)clang_getCursorKind(c_fn)) {
		case CXCursor_CXXMethod:
		case CXCursor_Constructor:
		case CXCursor_Destructor:
			return 1;
		default:
			return 0;
	}
}

/** does c_fn take an implicit `this` argument?
 *
 * a static C++ method has the method cursor kind but no `this`; a
 * constructor/destructor always has one. */
ae2f_inline_f static aclspv_bool_t aclutil_fn_has_this(CXCursor c_fn) {
	if(!aclutil_fn_is_method(c_fn))
		return 0;

	if(clang_getCursorKind(c_fn) == CXCursor_CXXMethod
			&& clang_CXXMethod_isStatic(c_fn))
		return 0;

	return 1;
}

/** can this body be emitted before every callee it names?
 *
 * aclsym_body_ready asks for a callee id. a call the inliner carves needs
 * none — the callee's body is carved right here — so e.g. an `always_inline`
 * callee may be defined after (or inside) its caller. everything else still
 * has to be emitted first. one policy applies at every call site of a given
 * callee, so the declaration's answer is the body's answer. */
struct s_aclutil_call_ready {
	h_aclspv_ctx_t	m_ctx;
	aclspv_bool_t	m_ready;
};

static enum CXChildVisitResult s_aclutil_call_ready_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr
		) {
	struct s_aclutil_call_ready* ae2f_restrict p = rdwr;

	if(c_cur.kind == CXCursor_CallExpr) {
		CXCursor		c_callee = clang_getCursorReferenced(c_cur);
		aclsym_fn_t* ae2f_restrict p_sym;

		if(!clang_Cursor_isNull(c_callee)
				&& (c_callee.kind == CXCursor_FunctionDecl
					|| aclutil_fn_is_method(c_callee))) {
			p_sym = aclsym_fn_find(c_callee, p->m_ctx);

			unless(p_sym)
				return CXChildVisit_Recurse;

			/** the module can name it. */
			if(p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_EMITTED))
				return CXChildVisit_Recurse;

			/** a defined callee the inliner will carve needs no id of
			 * its own. */
			if((p_sym->m_flag0 & aclsym_fn_flag0(ACLSYM_FN_FLAG0_DEFINED))
					&& aclattr_wants_inlining(c_callee, p->m_ctx))
				return CXChildVisit_Recurse;

			p->m_ready = 0;
			return CXChildVisit_Break;
		}
	}

	return CXChildVisit_Recurse;
}

ae2f_inline_f static aclspv_bool_t aclutil_body_calls_ready(
		CXCursor	c_body,
		h_aclspv_ctx_t	h_ctx
		) {
	struct s_aclutil_call_ready P;

	P.m_ctx		= h_ctx;
	P.m_ready	= 1;

	clang_visitChildren(c_body, s_aclutil_call_ready_visit, &P);
	return P.m_ready;
}

enum ACLUTIL_VARINIT_ {
	ACLUTIL_VARINIT_NONE = 0,
	ACLUTIL_VARINIT_CTOR,
	ACLUTIL_VARINIT_VALUE
};

/** classify an initializer by its cursor kind only.
 *
 * libclang maps CXXConstructExpr to CallExpr, so the referenced
 * constructor's copy/move predicates are the only extra information needed:
 *
 *   InitListExpr            -> value initialisation (`{}` / `= {}`)
 *   CallExpr + copy/move    -> value initialisation (`= other`)
 *   CallExpr + other ctor   -> constructor call (`{}` / default init)
 *   anything else           -> ordinary value initialisation
 *
 * `S s;` with no user-provided constructor is the implicit default ctor
 * CallExpr; the constructor call path then finds no symbol and does nothing.
 * */
ae2f_inline_f static enum ACLUTIL_VARINIT_ aclutil_varinit_kind(CXCursor c_init) {
	CXCursor	CTOR;

	switch((aclspv_cxcurkind_t)clang_getCursorKind(c_init)) {
		case CXCursor_InitListExpr:
			return ACLUTIL_VARINIT_VALUE;

		case CXCursor_CallExpr:
			CTOR = clang_getCursorReferenced(c_init);

			if(clang_getCursorKind(CTOR) != CXCursor_Constructor)
				return ACLUTIL_VARINIT_VALUE;

			if(clang_CXXConstructor_isCopyConstructor(CTOR)
					|| clang_CXXConstructor_isMoveConstructor(CTOR))
				return ACLUTIL_VARINIT_VALUE;

			return ACLUTIL_VARINIT_CTOR;

		default:
			return ACLUTIL_VARINIT_VALUE;
	}
}

static enum CXChildVisitResult s_aclutil_has_compound(
		CXCursor	c,
		ae2f_unused CXCursor p,
		CXClientData	rw
		) {
	(void)p;

	if(clang_getCursorKind(c) == CXCursor_CompoundStmt) {
		*((aclspv_bool_t*)rw) = 1;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}

/** does the constructor have a body in this translation unit?
 *
 * an implicit or explicitly-defaulted constructor has no CompoundStmt. this
 * frontend follows C for those: the declaration is left uninitialised. */
ae2f_inline_f static aclspv_bool_t aclutil_ctor_has_body(CXCursor c_ctor) {
	CXCursor		DEF = clang_getCursorDefinition(c_ctor);
	aclspv_bool_t		FOUND = 0;

	if(!clang_Cursor_isNull(DEF))
		c_ctor = DEF;

	clang_visitChildren(c_ctor, s_aclutil_has_compound, &FOUND);
	return FOUND;
}

/** the `std::move` declaration itself. */
ae2f_inline_f static aclspv_bool_t aclutil_fn_is_std_move_decl(CXCursor c_fn) {
	CXCursor		PARENT;
	CXString		SPELL, PNAME;
	const char*		c_spell;
	const char*		c_pname;
	aclspv_bool_t		R = 0;

	if(clang_getCursorKind(c_fn) != CXCursor_FunctionDecl)
		return 0;

	PARENT = clang_getCursorSemanticParent(c_fn);

	if(clang_getCursorKind(PARENT) != CXCursor_Namespace)
		return 0;

	SPELL = clang_getCursorSpelling(c_fn);
	PNAME = clang_getCursorSpelling(PARENT);
	c_spell = clang_getCString(SPELL);
	c_pname = clang_getCString(PNAME);

	R = (aclspv_bool_t)(c_spell && c_pname
			&& !strcmp(c_spell, "move")
			&& !strcmp(c_pname, "std"));

	clang_disposeString(SPELL);
	clang_disposeString(PNAME);

	return R;
}

/** `std::move(x)`: a transparent cast in this language model. */
ae2f_inline_f static aclspv_bool_t aclutil_cursor_is_std_move(CXCursor c_cur) {
	if(clang_getCursorKind(c_cur) != CXCursor_CallExpr)
		return 0;

	return aclutil_fn_is_std_move_decl(clang_getCursorReferenced(c_cur));
}

/** a copy/move constructor CallExpr is a value copy in this model, not a
 * call to an emitted constructor body. */
ae2f_inline_f static aclspv_bool_t aclutil_cursor_is_copy_move_ctor_call(CXCursor c_cur) {
	CXCursor	CTOR;

	if(clang_getCursorKind(c_cur) != CXCursor_CallExpr)
		return 0;

	CTOR = clang_getCursorReferenced(c_cur);

	if(clang_getCursorKind(CTOR) != CXCursor_Constructor)
		return 0;

	return (aclspv_bool_t)(clang_CXXConstructor_isCopyConstructor(CTOR)
			|| clang_CXXConstructor_isMoveConstructor(CTOR));
}

#endif
