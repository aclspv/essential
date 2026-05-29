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

#if 1
kernel_raw(0) void KERNEL(void) {
    goto K;
    double4    c, c1, c2;
    float    cc = 1;
    float4    d = cc;
    double4    a = d * cc;
K:
    c += a;
    c2 += c;
}
#endif

__attribute__((annotate("reqd_work_group_size(3, 4, 2)"))) /** work group size should be specified on constant time. */
kernel_raw(5) void N_KERNEL_0(
        __constant uintptr_t* consti32, 
        storage_class(3) const uint* u32out

        ) {
    btcast float a0 = 1078774989;
    char a1 specid(0);
    int a2 = a0 + (a0 += a1);
    btcast long a = (double)(int)(float)(a0 + a1 + a2);

    {
        if(0) { int B = 3; a2 += (B -= 2); }
        else if(a) { bratio(3, 3) int B = 4; a2 += B; if(1) { int C = 3; a2 += C; } }
        else {}
    }

#if 1
    while(a2 -= 1) {
        bratio(0, 0) int AAA;
        a1 += a2;
        break;
    }

    a2 += 8;
    a2 += 7.;
#endif

    goto GOTO;

    a2 += 6;
    a2 += 5;

GOTO:
    a2 += 3;
    a2 += 3;
    return;
}
