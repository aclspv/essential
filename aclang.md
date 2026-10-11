# aclang
> `aclang`, annotated C-family language parses subset of clang ast.  
> This page describes the pure specification of `aclang`, without preprocessing sugar.

# size of integer

|       | char | short | int | long |
| ----- | ---- | ----- | --- | ---- |
| bytes | 1    | 2     | 4   | 8    |

with unsigned is also possible.

# Attributes
> `__attribute__((ext_vector_type(a)))`

> make a vector type.  
> a vector type is a simd type, where many operations maps to simd.  

```c
typedef int i32;
typedef i32 __attribute__((ext_vector_type(4))) v4i32;

_Bool band128(const v4i32 A, const v4i32 B) {
    v4i32 Z;
    Z[0] = 0; Z[1] = 0;
    Z[3] = 0; Z[4] = 0;

    return A & B != Z;  
}
```

# AnnotationAttributes
> Some compiler attributes are available for vulkan-specific behaviours.  
> Many will be annotation attribute so seem as `__attribute__((annotate("<here>")))`  
> arguments should be literal. it is not done by travelling ast.

### aclspv_specid

> `aclspv_specid(specid : u32)`

> Make constant value modifiable on vulkan pipeline.  
> See [shader stage create info](https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineShaderStageCreateInfo.html)  

- if `specid` is `0xFFFFFFFF`, it will be seen undefined.
- Maps to `OpSpecConstant` with id of `specid`
- Result type must be scalar Integer or Floating point type.
- Follows the first specified value & data type.
- Default value must be specified on first use.
- Ignores the initialising value when specid is found.
- Must be specified on use. E.g:
    - Setting a size of an constant array
    - Initialising a non-constant variable
- Specifying on following condition will be ignored
    - Enumerators
    - __constant variable
    - Constant literals
```c

typedef int i32;

/** 
 * r's size will be determined by spec constant with id 0.
 * without constant specified in certain type, spec constant will be defined with a default of 3.
 * */
i32 sum(const i32 rd[3] __attribute__((annotate("aclspv_specid(0)")))) {

    /** since specid is specified upon, no specifying default value is required. */
    i32 c __attribute__((annotate("aclspv_specid(0)")));
    i32 ret = 0;
    
    for(i32 i = 0; i < c; ++i)
        ret += rd[i];

    return ret;
}

```

### aclspv_execmodel
> `aclspv_execmodel(model : u32)`

- make a function entry point
- only attach this on a function which returns void. others should be ignored.
- possible inputs are defined as enum `SpvExecutionModel_` on spirv-header.

```c

/** this is a normal function */
void normal_function(void) {
    return 0;
}

/**
 * this is an entry point of vertex. (0 is a number for vertex execution model)
 */
__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(void) {
    return;
}

```

entry point  
> is a void (void); with interface attached.  
> you can have multiple entry points in a shader. (you can have vertex & frag shader at one go!)  
> in aclang the entry point is very flexible since execution mode & objects are represented as attributes & arguments.

arguments  
> unlike normal function, arguments of entry point have different meaning.    
> for pointers with [storage class](#aclspv_storage_class) will be a variable declared globally binded.  
> for non-pointers will be an element appended on one push constant block.  
> without `storage class` specified on pointer parameter, compiler should emit an error.  

bindables  
> every entry points have its own reserved [descriptor set](#aclspv_set), incremented from 0.  
> from left to right, bindable arguments have its reserved [binding index](#aclspv_bind) from 0, incremented on every bindable obejcts.  

```c

__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(
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
    __attribute__((annotate("aclspv_storage_class(12)"))) struct {int c; }* sbo_struct /** struct { int c; }  } */,

    /***
     * 2: uniform buffer
     * bindable, which should follow std140 memory layout.
     */

    /** bind number 2 **/
    __attribute__((annotate("aclspv_storage_class(2)"))) int* ubo_int,
    
    /** bind number 3 **/
    __attribute__((annotate("aclspv_layout(1)")))
    __attribute__((annotate("aclspv_storage_class(2)")))
    struct { int c; }* ubo_struct,

    /*** 4: workgroup */ 
    __attribute__((annotate("aclspv_storage_class(4)"))) int* work_int,
    __attribute__((annotate("aclspv_storage_class(4)"))) int work_i32_15[15],
    __attribute__((annotate("aclspv_storage_class(4)"))) int work_int_arr_spec[] __attribute__((annotate("aclspv_specid(1)")))
) {
    return;
}

```

### aclspv_storage_class

> `aclspv_storage_class(class : u32)`

> Specifies the storage class for the target pointer type.  
> Possible inputs are defined as enum `SpvStorageClass_` on spirv-header.  
> pointer type with same element type and different storage class is considered different pointer type.

```c
__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(
    /*** 
     * 12: storage buffer
     * */
    __attribute__((annotate("aclspv_storage_class(12)"))) int sbo_int_rtarray[] /** struct { runtimearray<int> a; } */
) {
    /** legal, has same storage class. */
    __attribute__((annotate("aclspv_storage_class(12)"))) int* p0 = &sbo_int_rtarray[3];

    /** illegal, has different storage class. */
    __attribute__((annotate("aclspv_storage_class(5)"))) int* p1 = &sbo_int_rtarray[3];
}
```

you can typedef it.

```c
typedef unsigned int u32;
typedef u32* __attribute__((annotate("aclspv_storage_class(12)"))) p_u32_sbo_t;
typedef struct { u32 m_lsb; u32 m_hsb; } u32x2;
typedef u32x2* __attribute__((annotate("aclspv_storage_class(12)"))) p_u32x2_sbo_t;
```

### aclspv_set
> `aclspv_set(set : u32)`

- specifies the descriptor sets of bindable object.  
- could be used on bindable objects.  
- when specified, specifing [binding index](#aclspv_bind_idx) is required.

### aclspv_bind_idx
> `aclspv_bind_idx(idx : u32)`

- specifies the binding index of bindable object.

### aclspv_wrkgroup_size
> `aclspv_wrkgroup_size(x: u32, y: u32, z: u32)`  
> `reqd_work_group_size(x: u32, y: u32, z: u32)`

- specifies the entry point's workgroup size.
- `x`, `y`, `z` must be a 32-bit integer as decimal literal.
- This attribute is ignored when its execution model is not either `Kernel(6)` or `GLCompute(5)`.

```c
__attribute__((annotate("aclspv_wrkgroup_size(1, 1, 1)")))  /** with workgroup size (1, 1, 1) */
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute */
void entry_point_glcompute(void) {
    return;
}
```

### aclspv_wrkgroup_size_id
> [!NOTE]  
> this attribute is available from spir-v version 1.2.

> `aclspv_wrkgroup_size_id(x : u32 | id, y: u32 | id, z: u32 | id)`

- specifies the entry point's workgroup size, with not only literal, but an spec constant id.
- to specify which is literal or specid, prefix is used.

```c
/**
 * & means it is spec constant id.
 * # means it is literal.
 * 
 * therefore following line means it size should be
 * x: specified constant in id 1
 * y: 3
 * z: specified constant in id 2
 */
__attribute__((annotate("aclspv_wrkgroup_size_id(&1, #3, &2)")))
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute */
void entry_point_glcompute(void) {
    return;
}
```

### aclspv_location

> `aclspv_location(location : u32)`

- used on declaring arguments in entry point.
- specifies the location of `input(1)` or `output(3)`  
- when not specified, every input our output's location will be 0.  

```c
__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(
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
```

### aclspv_btcast

> `aclspv_btcast`

- tells that the final casting of variable would be bit-casting when it is arithmetic casting.

```c
void foo(void) {
    /** 3.20000005 */
    __attribute__((annotate("aclspv_btcast"))) float bar = 1078774989;
}
```

### aclspv_bratio

> `aclspv_bratio(weight_true_branch : u32, weight_false_branch : u32)`  

- declare a branch ratio of possibility that whether branch would be true or not.  
- this attribute is a performance hint which affects the logic-based control statement has two cases
- while
- if

```c
typedef int i32;
struct foo { i32 m; };

i32 foo_read_32(struct foo* h_whatever) {
    if(h_whatever) {    /** weighted as 7 */
        __attribute__(("aclspv_bratio(7, 1)")) typedef int dummy_t;
        return h_whatever->m;
    } else {            /** weighted as 1 */
        return 0;
    }
}
```

### aclspv_layout
> `aclspv_layout(layout_number: u32)`  

- specify a variable | type be follows a certain memory layout  
- see `./lib/aclty/layout.h` to get the layouts  
- without this attribute specified, layout will be default. (see ACLSPV_LAYOUT_)

```c
__attribute__((annotate("aclspv_layout(1)")))
typedef struct { unsigned m_uint; } std140_uint_t;
```

# Restriction
> Some statements / extensions may not be planned to be supported for now.

- `VK_KHR_relaxed_block_layout` behaviour (free alignment)  
- Non plain-old-data types  
- `OpenCL` memory model  
- `function-scoped variable` shadowing function parameter  
- designated initializer list
- pointer arithmetic on non-physical addressing model
- global variables
