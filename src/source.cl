typedef int i32;
typedef i32 __attribute__((ext_vector_type(4))) i32x4;
typedef _Bool __attribute__((ext_vector_type(4))) boolx4;

#if	1

boolx4 t0_band128(const i32x4 A, const i32x4 B) {
    i32x4 Z;
    Z[0] = 0; Z[1] = 0;
    Z[2] = 0; Z[3] = 0;

    Z *= 15;

    return A & B != Z;
}

#endif

typedef int i32;

#if	1

/**
 * r's size will be determined by spec constant with id 0.
 * without constant specified in certain type, spec constant will be defined with a default of 3.
 * */
i32 t1_sum(const i32 rd[3] __attribute__((annotate("aclspv_specid(0)")))) {

    /** since specid is specified upon, no specifying default value is required. */
    i32 c __attribute__((annotate("aclspv_specid(0)")));
    i32 ret = 0;

    for(i32 i = 0; i < c; ++i)
        ret += rd[i];

    return ret;
}

#endif

#if	1

/** this is a normal function */
void t2_norm_func(void) {
    return;
}

#endif

#if	1

/**
 * this is an entry point of vertex. (0 is a number for vertex execution model)
 */
__attribute__((annotate("aclspv_execmodel(0)")))
void t3_entry_point_vertex(void) {
    return;
}

#endif

#if	1

__attribute__((annotate("aclspv_execmodel(0)")))
void t4_entry_point_vertex(
    int pc_int /** becomes a push constant's element. follows std430 memory layout. */,

    /*** 3: output */
    __attribute__((annotate("aclspv_storage_class(3)")))
    int* out_int,

    /***
     * 12: storage buffer
     * bindable, which should follow std430 memory layout.
     * */

    /** bind number 0 **/
    __attribute__((annotate("aclspv_storage_class(12)"))) int* sbo_int_rtarray /** struct { runtimearray<int> a; } */,
    /** bind number 1 **/
    __attribute__((annotate("aclspv_storage_class(12)"))) struct { int c; }* sbo_struct /** struct { int c; }  */,

    /***
     * 2: uniform buffer
     * bindable, which should follow std140 memory layout.
     * */

    /** bind number 2 **/
    __attribute__((annotate("aclspv_storage_class(2)"))) int* ubo_int,

    /** bind number 3 **/
    __attribute__((annotate("aclspv_layout(1)")))
    __attribute__((annotate("aclspv_storage_class(2)")))
    struct { int c; }* ubo_struct,

    /*** 4: workgroup */
    __attribute__((annotate("aclspv_storage_class(4)"))) int* work_int,
    __attribute__((annotate("aclspv_storage_class(4)"))) int work_i32_15[15],
    __attribute__((annotate("aclspv_storage_class(4)"))) int work_int_arr_spec[] __attribute__((annotate("aclspv_specid(1)"))),

    int pc2
) {
	sbo_struct->c = 3;
	(sbo_struct)->c = 4;
	(sbo_struct->c) = 5;
	return;
}

#endif

#if	1

__attribute__((annotate("aclspv_execmodel(0)")))
void t5_entry_point_vertex(
    /***
     * 12: storage buffer
     * */
    __attribute__((annotate("aclspv_storage_class(12)"))) int sbo_int_rtarray[] /** struct { runtimearray<int> a; } */
) {
    /** legal, has same storage class. */
    __attribute__((annotate("aclspv_storage_class(12)"))) int* p0 = &sbo_int_rtarray[3];

    #if	0
    /** illegal, has different storage class. */
    __attribute__((annotate("aclspv_storage_class(5)"))) int* p1 = &sbo_int_rtarray[3];
    #endif
}

#endif

#if	1

typedef unsigned int u32;
typedef u32* __attribute__((annotate("aclspv_storage_class(12)"))) p_u32_sbo_t;
typedef struct { u32 m_lsb; u32 m_hsb; } u32x2;
typedef u32x2* __attribute__((annotate("aclspv_storage_class(12)"))) p_u32x2_sbo_t;

#endif

#if	1

__attribute__((annotate("aclspv_wrkgroup_size(1, 1, 1)")))  /** with workgroup size (1, 1, 1) */
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute */
void t6_entry_point_glcompute(void) {
    return;
}

#endif

#if	0
__attribute__((annotate("aclspv_wrkgroup_size_id(&1, #3, &2)")))
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute */
void t7_entry_point_glcompute(void) {
    return;
}
#endif

#if	1

__attribute__((annotate("aclspv_execmodel(0)")))
void t8_entry_point_vertex(
    /***
     * omitted.
     * location: 0
     * */
    __attribute__((annotate("aclspv_storage_class(1)"))) int* in_int,

    /**
     * location: 1
     **/
    __attribute__((annotate("aclspv_location(1)")))
    __attribute__((annotate("aclspv_storage_class(3)"))) int* out_int
) { *out_int = *in_int; }

#endif

#if	1

__attribute__((annotate("aclspv_execmodel(0)")))
void t9_foo(__attribute__((annotate("aclspv_storage_class(3)"))) float* out) {
    /** 3.20000005 */
    __attribute__((annotate("aclspv_btcast"))) float bar = 1078774989;
    *out = bar;
}

#endif

#if	1

typedef int i32;

__attribute__((annotate("aclspv_execmodel(0)")))
void t10_foo_read_32(
		     __attribute__((annotate("aclspv_storage_class(1)"))) int* h_whatever,
		     __attribute__((annotate("aclspv_storage_class(3)"))) int* out
		     ) {
    if(*h_whatever) {    /** weighted as 7 */
        __attribute__(("aclspv_bratio(7, 1)")) typedef int dummy_t;
        *out = *h_whatever;
    } else {            /** weighted as 1 */
        *out = 0;
    }

    int a;

    #if 1
    int* p = (&(a));
    a = 3;
    (*p) = 2;
    *(p) = 5;
    #endif
    a = 4;
}

#endif

#if	1

__attribute__((annotate("aclspv_layout(1)")))
typedef struct { unsigned m_uint; } std140_uint_t;

#endif

#if 1
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute */
void t11_dedup_entry(
		     __attribute__((annotate("aclspv_storage_class(4)"))) int dedup_a[15]
		     ) {
	dedup_a[0] = (dedup_a)[0] + dedup_a[0];
	dedup_a[2] = ((((dedup_a))))[(3)] + dedup_a[4];
	(dedup_a)[0] = 9;
	((((dedup_a))))[(1)] = 8;
}
#endif

#if	1
typedef float f32;
typedef f32 __attribute__((ext_vector_type(4))) f32x4;

__attribute__((annotate("aclspv_execmodel(0)")))
void t12_entry_point_vertex(
    /***
     * vertex shader builtin: position output
     * SPIR-V BuiltIn: Position (0)
     * */
    __attribute__((annotate("aclspv_storage_class(3)")))
    __attribute__((annotate("aclspv_bltin(0)")))
    f32x4* out_pos,

    /***
     * vertex shader builtin: vertex index input
     * SPIR-V BuiltIn: VertexIndex (42)
     * */
    __attribute__((annotate("aclspv_storage_class(1)")))
    __attribute__((annotate("aclspv_bltin(42)")))
    int* in_vertidx
) { return; }

#endif
