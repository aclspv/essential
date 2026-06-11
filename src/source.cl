/** 
 * This source is test experimental.
 * extensions will be attributes inside.
 * 
 * macros will be defined in opencl-base.h
 **/

typedef vector(4) float float4;
typedef vector(4) double double4;
typedef vector(4) long long4;
typedef vector(4) int int4;
typedef vector(4) _Bool bool4;

#if 0
kernel_raw(0) void KERNEL(void) {
    double4    c, c1, c2;
    float    cc = 2;
    float4    d = cc;
    double4    a = d * cc;

    if((int)cc) {}

    return;
}
#endif

__attribute__((annotate("reqd_work_group_size(3, 4, 2)"))) /** work group size should be specified on constant time. */
kernel_raw(5) void N_KERNEL_0(
        __constant uintptr_t* consti32, 
        storage_class(3) const uint* u32out

        ) {
    btcast float a0 = 1078774989;
    float a1 = (a0 += 3.f) * 2;
    float a2;

    a2 -= a1;

    if((int)a1) {  } else {}

    return;
}
