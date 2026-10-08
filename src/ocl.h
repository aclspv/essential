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

#endif
