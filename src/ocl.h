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

/**
 * @brief base opencl c header this compiler supports
 * */
#ifndef ACLSPV_H
#define ACLSPV_H

#ifndef ACLSPV_PTR64
#define ACLSPV_PTR64	1
#endif

#define	ACLSPV_I(t,b)	typedef t i##b;typedef unsigned t u##b, u##t
ACLSPV_I(char,8);ACLSPV_I(short,16);ACLSPV_I(int,32);ACLSPV_I(long,64);
#undef	ACLSPV_I


#define f16	half
#define f32	float
#define	f64	double
#define	uptr	size_t
#define	iptr	ptrdiff_t
#define intptr_t	iptr
#define uintptr_t	uptr

#if	ACLSPV_PTR64
typedef	u64 uptr;typedef i64 iptr;
#else
typedef	u32 uptr;typedef i32 iptr;
#endif

#define	aclattr(a)	__attribute__((annotate(a)))
#define	vector(a)	__attribute__((ext_vector_type(a))) 
#define	kernel_raw(a)	aclattr("aclspv_execmodel( " #a " )")
#define	storage_class(a)	aclattr("aclspv_storage_class( " #a " )")

#define	btcast		aclattr("aclspv_btcast")
#define specid(a)	aclattr("aclspv_specid( " #a " )")
#define	bratio(t, f)	aclattr("aclspv_bratio ("#t","#f")")
#define layout(layout)	aclattr("aclspv_layout(" #layout ")")

#if	defined(__OPENCL_VERSION__) && __OPENCL_VERSION__
#define	__kernel	kernel_raw(5)		__kernel
#define	__global	storage_class(12)	__global
#define __constant	storage_class(2)	__constant
#define	__local		storage_class(4)	__local
#else
#define	__kernel	kernel_raw(5)	
#define	__global	storage_class(12)
#define __constant	storage_class(2)
#define	__local		storage_class(4)
#endif

/**
 * @name synchronisation builtins
 *
 * they have no definition on purpose: the compiler recognises the call by
 * spelling and emits the instruction in place, so no symbol is ever created
 * and no body is ever looked for.
 *
 * the arguments are scope / memory-semantics words, not expressions: each one
 * must be an integer constant expression, and its numbering belongs to the
 * memory model the context declares (GLSL450 uses `SpvScope*` and
 * `SpvMemorySemantics*Mask`; Vulkan uses `SpvScope*KHR`).
 *
 * both return void: a barrier is a statement, not a value.
 * @{
 * */

/** OpControlBarrier: synchronise the execution and memory domains.
 *
 * @param scope_exec	SpvScope_: who has to reach this point
 * @param scope_mem	SpvScope_: the memory any following control-dependent
 *			access observes under the semantics below
 * @param mem_sem	SpvMemorySemanticsMask: which memories and which
 *			flavours of ordering
 * */
void aclspv_ctlbar(u32 scope_exec, u32 scope_mem, u32 mem_sem);

/** OpMemoryBarrier: order this invocation's memory accesses against every
 * memory access in `scope_mem`.
 * */
void aclspv_membar(u32 scope_mem, u32 mem_sem);

/** @} */

#endif
