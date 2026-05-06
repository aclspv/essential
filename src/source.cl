/** 
 * This source is test experimental.
 * extensions will be attributes inside.
 * 
 * macros will be defined in opencl-base.h
 **/

#define	double float

typedef vector(4) float float4;

kernel_raw(0) void KERNEL() {
	float4 a; 
	float4 b;
	a += b;
	goto L0;

L0:
	a += b;
	return;
}

__attribute__((annotate("reqd_work_group_size(2, 2, 2)"))) /** work group size should be specified on constant time. */
kernel_raw(0) void N_KERNEL_0(
__constant uintptr_t* consti32, 
storage_class(3) const uint* u32out

) {
	btcast float a0 = 1078774989;
	char a1 
#if 0
		= 1
#else
		specid(0)
#endif
			;
	int a2 = a0 + (a0 += a1);
#if 1
	btcast long a = (double)(int)(float)(a0 + a1 + a2);

	if(0) { int B = 3; a2 += (B -= 2); } 
	else if(a) { bratio(3, 3) int B = 4; a2 += B; if(1) { int C = 3; a2 += C; } }  else {}

	while(a2 -= 1) {
		bratio(0, 0) int AAA;
		a1 += a2;
		break;
	}

	a2 += 8;
	a2 += 8;
#endif

	goto GOTO;

	a2 += 8;
	a2 += 8;

GOTO:
	a2 += 3;
	a2 += 3;
	return;
}
