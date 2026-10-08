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

#if	1
void NON_KERNEL(int a) {
	int A;
	A += 3;
}

void NON_KERNEL2(int a) {
	int A = 3;
	A += 3;
}
#endif

#if 1
kernel_raw(0) void KERNEL(int aaaa, int a2) {
	goto K;
	double4    c, c1, c2;
	float    cc = 1;
	float	bb;
	float4    d = cc;
	double4    a  = d * cc;

K:
	{
		double4    c, c1, c2;
		c += a;
		c2 += c;
	}

	if(bb) {}
}

kernel_raw(0) void BOOLEAN(float4 float4_boolean) {
	_Bool OP0;
	_Bool OP1;

	_Bool OP_OR;

	if(OP_OR) {  }

	OP_OR		= OP0 || OP1;
	_Bool OP_AND	= OP0 && OP1;
}
#endif

__attribute__((annotate("reqd_work_group_size(3, 4, 2)"))) /** work group size should be specified on constant time. */
kernel_raw(5) void N_KERNEL_0(
		__constant uintptr_t* consti32, 
		storage_class(3) uint* u32out,
		__local float i32aaaa[3],
		layout(1) __constant float i32aaab[3],
		const int pshconst
		) {

	btcast float a0 = 1078774989;
	int a1 specid(0) = 3;
	int a2 = a0 + (a0 += a1);
	int dd = 3 && 0;

	btcast long a = (double)(int)(float)(a0 + a1 + a2);

#if 1
	while(a2++ & a1) {
		bratio(1, 2) int AAA;
		a1 += a2;

		if(0) { int B = 3; a2 += (B -= 2); continue; }
		else if(a) { bratio(3, 3) int B = 4; a2 += B; if(1) { int C = 3; a2 += C; } }
		else {
			break;
		}
	}

#else
	int x = 0, y = 7;
	while (x < 10) {
		if (x & 1)
			y += x;
		else
			y += 2;

		x++;
	}
#endif

	a2 = 8;
#if 	0
	goto GOTO;

	a2 += 6;
	a2 += 5;

GOTO:
#endif
	a2 += 3;
	a2 += 3;

	{
		i32 a3;
		a3 <<= 3;
		a3 >>= 4;
		a3 = a3 << 4 >> a3;
	}

	{
		u32 a3;
		u32 a4;
		a3 <<= 3;
		a3 >>= 4;
		a3 = a4 = a3 << 4;
	}

	{
		union L2ST {
			int a;
			float b;
		};

		struct L2NEST {
			int c;
			union L2ST st;
		};

		struct L2VM {
			float4 v;
		};

		struct L2AM {
			int arr[3];
		};

		/** lane 2: struct aggregate initializers */
		union L2ST s = { 1 };
		struct L2NEST t = { 9, { 3, 4.5f } };
		struct L2VM vm = { (float4){ 1.f, 2.f, 3.f, 4.f } };
		struct L2AM am = { { 5, 6, 7 } };
		struct L2AM am2;

		am = am2;

		/** lane 3: composite member reads */
		float a2;
		a2 = s.b;
		int c2;
		c2 = t.c;

		/** lane 4: composite member writes (lvalue targets) */
		s.b = 6.0f;
		t.c = 8;
		c2 = t.c;
	}

	{
		int a = 3;
		int* p = &a;
		int* q = &a;

		*p = 2;
		*q += 2;
	}

	{
		int idx;
		int a[3];
		int* p2 = &a[idx - 1];
		if((a[idx] = *p2) - 5) { a[0] = 2; };
	}

	return;
}

