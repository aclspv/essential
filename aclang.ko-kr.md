# aclang

> `aclang`은 Clang AST의 부분 집합을 파싱하는 주석이 포함된 C 계열 언어입니다.  
> 이 페이지는 `aclang`의 전처리 문법 설탕을 제외한 순수한 명세를 설명합니다.  
> 나열된 패턴은 유효한 SPIR-V로 출력될 수 있습니다.

## size of integer

정수 타입의 크기

|       | char | short | int | long |
| ----- | ---- | ----- | --- | ---- |
| bytes | 1    | 2     | 4   | 8    |

`unsigned` 타입도 사용할 수 있습니다.

## Attributes

속성

> `__attribute__((ext_vector_type(a)))`  
>
> 벡터 타입을 만듭니다.  
> 벡터 타입은 SIMD 타입이며, 여러 연산이 SIMD 연산으로 매핑됩니다.

```c
typedef int i32;
typedef i32 __attribute__((ext_vector_type(4))) v4i32;
typedef _Bool __attribute__((ext_vector_type(4))) boolx4;

boolx4 band128(const v4i32 A, const v4i32 B) {
    v4i32 Z;
    Z[0] = 0; Z[1] = 0;
    Z[2] = 0; Z[3] = 0;

    return A & B != Z;  
}
```

## AnnotationAttributes

주석 속성

> Vulkan 관련 동작을 위한 일부 컴파일러 속성이 제공됩니다.  
> 상당수는 어노테이션 속성일 것이며, 따라서 `__attribute__((annotate("<here>")))`와 같은 형태로 나타날 것입니다.  
> 인수는 리터럴 형태여야 합니다. AST를 순회하는 방식으로는 처리되지 않습니다.

### aclspv_specid

> `aclspv_specid(specid : u32)`
>
> Vulkan 파이프라인에서 상수 값을 수정 가능하게 만듭니다.  
> [셰이더 단계 생성 정보](https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineShaderStageCreateInfo.html)를 참조하십시오.

- `specid`가 `0xFFFFFFFF`인 경우, 이는 정의되지 않은 것으로 간주됩니다.
- ID가 `specid`인 `OpSpecConstant`에 ​​매핑됩니다.
- 결과 타입은 스칼라 정수타입 또는 부동소수점 타입이어야 합니다.
- 처음에 지정된 값 및 데이터 유형을 따릅니다.
- 처음 사용할 때 기본값을 반드시 지정해야 합니다.
- 지정된 값이 발견되면 초기화 값은 무시됩니다.
- 사용 시 명시해야 합니다. 예:
  - 상수 배열의 크기 설정
  - 비상수 변수 초기화
- 다음 항목에 대해서는 지정이 무시됩니다.
  - 열거자(enumerator)
  - `__constant` 변수
  - 상수 리터럴

```c

typedef int i32;

/** 
 * r의 크기는 ID가 0인 상수(spec constant)에 의해 결정됩니다.
 * 특정 타입으로 명시되지 않은 상수의 경우, 해당 상수(spec constant)는 기본값 3으로 정의됩니다.
 * */
i32 sum(const i32 rd[3] __attribute__((annotate("aclspv_specid(0)")))) {

    /** specid가 지정되어 있으므로, 기본값을 별도로 지정할 필요는 없습니다. */
    i32 c __attribute__((annotate("aclspv_specid(0)")));
    i32 ret = 0;
    
    for(i32 i = 0; i < c; ++i)
        ret += rd[i];

    return ret;
}

```

### aclspv_execmode

> `aclspv_execmode0(mode : u32)`  
> `aclspv_execmode1(mode : u32, o0 : u32 | id)`  
> `aclspv_execmode2(mode : u32, o0 : u32 | id, o1 : u32 | id)`  
> `aclspv_execmode3(mode : u32, o0 : u32 | id, o1 : u32 | id, o2 : u32 | id)`

<!-- -->

> [!WARNING]  
> 실행 모드(execution mode)에서 특수 상수 ID(spec constant ID)를 사용하려면 SPIR-V 1.2 이상이 필요합니다.
> 이들은 기능(capabilities) 및 확장(extensions)을 자동으로 해결하지 않습니다.

<!-- -->

> [!NOTE]  
>
> 로컬 크기는 전용 속성으로 지정할 수도 있습니다. 아래를 참조하세요.  
>
> - [aclspv_wrkgroup_size](#aclspv_wrkgroup_size)  
> - [aclspv_wrkgroup_size_id](#aclspv_wrkgroup_size_id)

- 인수를 사용해 진입점 함수에 실행 모드를 지정합니다.
- `mode`는 [실행 모드 번호](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#Execution_Mode)를 나타냅니다.
- 실행 모드에는 추가 피연산자가 있을 수 있습니다. 최대 세 개까지 확인했으므로 대부분의 경우 `aclspv_execmode3`로 충분합니다.

```c
/**
 * &는 특수 상수 ID(id)를 나타냅니다.
 * #은 리터럴(u32)을 나타냅니다.
 * 
 * 따라서 아래 선언은 크기가 다음과 같음을 나타냅니다.
 * o0: ID 1로 지정된 상수
 * o1: 3
 * o2: ID 2로 지정된 상수
 */
__attribute__((annotate("aclspv_execmode3(17, &1, #3, &2)")))
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute 실행 모델 */
void entry_point_glcompute(void) {
    return;
}

```

### aclspv_execmodel

> `aclspv_execmodel(model : u32)`

- 함수를 진입점으로 선언합니다.
- 반환 타입이 `void`인 함수에만 지정할 수 있습니다. 그 외 함수에 지정하면 무시해야 합니다.
- 사용할 수 있는 값은 spirv-header의 `SpvExecutionModel_` 열거형에 정의되어 있습니다.

```c

/** 일반 함수입니다. */
void normal_function(void) {
    return;
}

/**
 * 정점 셰이더의 진입점입니다. (0은 정점 실행 모델의 값입니다.)
 */
__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(void) {
    return;
}

```

진입점  
> 인터페이스가 연결된 `void (void)` 함수입니다.  
> 하나의 셰이더에 여러 진입점을 둘 수 있습니다. (정점 셰이더와 프래그먼트 셰이더를 함께 둘 수 있습니다.)  
> `aclang`에서는 실행 모드와 객체를 속성 및 인수로 표현하므로 진입점을 유연하게 구성할 수 있습니다.

인수  
> 일반 함수와 달리 진입점의 인수에는 특별한 의미가 있습니다.  
> [저장 클래스](#aclspv_storage_class)가 지정된 포인터는 전역으로 선언되고 바인딩되는 변수가 됩니다.  
> 포인터가 아닌 인수는 하나의 푸시 상수 블록에 추가되는 요소가 됩니다.  
> 포인터 매개변수에 `storage class`가 지정되지 않으면 컴파일러는 오류를 발생시켜야 합니다.

바인딩 가능한 객체  
> 각 진입점에는 0부터 증가하는 전용 [디스크립터 세트](#aclspv_set)가 예약됩니다.  
> 왼쪽에서 오른쪽 순서로 각 바인딩 가능한 인수에 [바인딩 인덱스](#aclspv_bind_idx)가 0부터 차례로 할당됩니다.

```c

__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(
    int pc_int /** 푸시 상수의 요소가 되며 std430 메모리 레이아웃을 따릅니다. */,

    /*** 3: 출력 */
    __attribute__((annotate("aclspv_storage_class(3)")))
    int* out_int,

    /*** 
     * 12: 스토리지 버퍼
     * 바인딩 가능한 객체이며 std430 메모리 레이아웃을 따라야 합니다.
     * */

    /** 바인딩 번호 0 **/
    __attribute__((annotate("aclspv_storage_class(12)"))) int* sbo_int_struct /** struct { int a; } */,
    /** 바인딩 번호 1 **/
    __attribute__((annotate("aclspv_storage_class(12)"))) struct {int c; }* sbo_struct /** struct { int c; }  } */,

    /***
     * 2: 유니폼 버퍼
     * 바인딩 가능한 객체이며 std140 메모리 레이아웃을 따라야 합니다.
     */

    /** 바인딩 번호 2 **/
    __attribute__((annotate("aclspv_storage_class(2)"))) int* ubo_int,
    
    /** 바인딩 번호 3 **/
    __attribute__((annotate("aclspv_layout(1)")))
    __attribute__((annotate("aclspv_storage_class(2)")))
    struct { int c; }* ubo_struct,

    /*** 4: 워크그룹 */
    __attribute__((annotate("aclspv_storage_class(4)"))) int* work_int,
    __attribute__((annotate("aclspv_storage_class(4)"))) int work_i32_15[15],
    __attribute__((annotate("aclspv_storage_class(4)"))) int work_int_arr_spec[] __attribute__((annotate("aclspv_specid(1)")))
) {
    return;
}

```

### aclspv_storage_class

> `aclspv_storage_class(class : u32)`
>
> 대상 포인터 타입의 저장 클래스를 지정합니다.  
> 사용할 수 있는 값은 spirv-header의 `SpvStorageClass_` 열거형에 정의되어 있습니다.  
> 요소 타입이 같더라도 저장 클래스가 다르면 서로 다른 포인터 타입으로 취급됩니다.

```c
__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(
    /*** 
     * 12: 스토리지 버퍼
     * */
    __attribute__((annotate("aclspv_storage_class(12)"))) int sbo_int_rtarray[] /** struct { runtimearray<int> a; } */
) {
    /** 저장 클래스가 같으므로 유효합니다. */
    __attribute__((annotate("aclspv_storage_class(12)"))) int* p0 = &sbo_int_rtarray[3];

    /** 저장 클래스가 다르므로 유효하지 않습니다. */
    __attribute__((annotate("aclspv_storage_class(5)"))) int* p1 = &sbo_int_rtarray[3];
}
```

`typedef`로 별칭을 만들 수도 있습니다.

```c
typedef unsigned int u32;
typedef u32* __attribute__((annotate("aclspv_storage_class(12)"))) p_u32_sbo_t;
typedef struct { u32 m_lsb; u32 m_hsb; } u32x2;
typedef u32x2* __attribute__((annotate("aclspv_storage_class(12)"))) p_u32x2_sbo_t;
```

### aclspv_set

> `aclspv_set(set : u32)`

- 바인딩 가능한 객체의 디스크립터 세트를 지정합니다.
- 바인딩 가능한 객체에 사용할 수 있습니다.
- 이 속성을 지정하면 [바인딩 인덱스](#aclspv_bind_idx)도 지정해야 합니다.

```c
__attribute__((annotate("aclspv_execmodel(5)")))
void entry_point_compute(
    /*** 
     * 디스크립터 세트 2, 바인딩 0
     * */
    __attribute__((annotate("aclspv_storage_class(12)")))
    __attribute__((annotate("aclspv_set(2)")))
    __attribute__((annotate("aclspv_bind_idx(0)")))
    struct { int data; }* sbo_data,

    /*** 
     * 디스크립터 세트 2, 바인딩 1 (bind_idx를 지정하지 않아 0부터 자동 증가)
     * */
    __attribute__((annotate("aclspv_storage_class(12)")))
    __attribute__((annotate("aclspv_set(2)")))
    struct { float value; }* sbo_value
) { return; }
```

### aclspv_bind_idx

> `aclspv_bind_idx(idx : u32)`

- 바인딩 가능한 객체의 바인딩 인덱스를 지정합니다.
- 바인딩 가능한 객체에 사용할 수 있습니다.
- 이 속성을 지정하면 다음 객체의 자동 바인딩 인덱스 계산에서 이 인덱스를 기준으로 증가합니다.

```c
__attribute__((annotate("aclspv_execmodel(5)")))
void entry_point_compute(
    /*** 
     * 바인딩 5를 명시합니다. 같은 세트의 다음 바인딩 가능 객체는 6부터 시작합니다.
     * */
    __attribute__((annotate("aclspv_storage_class(12)")))
    __attribute__((annotate("aclspv_set(0)")))
    __attribute__((annotate("aclspv_bind_idx(5)")))
    struct { double x; }* sbo_explicit,

    /*** 
     * 바인딩 6 (앞에서 bind_idx를 지정했으므로 5에서 자동 증가)
     * */
    __attribute__((annotate("aclspv_storage_class(12)")))
    __attribute__((annotate("aclspv_set(0)")))
    struct { double y; }* sbo_auto
) { return; }
```

### aclspv_wrkgroup_size

> `aclspv_wrkgroup_size(x: u32, y: u32, z: u32)`  
> `reqd_work_group_size(x: u32, y: u32, z: u32)`

- 진입점의 워크그룹 크기를 지정합니다.
- `x`, `y`, `z`는 32비트 정수의 십진수 리터럴이어야 합니다.
- 실행 모델이 `Kernel(6)` 또는 `GLCompute(5)`가 아니면 이 속성은 무시됩니다.
- 지정된 진입점에 실행 모드를 추가합니다. 다른 실행 모드를 지정하려면 [실행 모드](#aclspv_execmode)를 참조하세요.

```c
__attribute__((annotate("aclspv_wrkgroup_size(1, 1, 1)")))  /** 워크그룹 크기는 (1, 1, 1)입니다. */
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute 실행 모델 */
void entry_point_glcompute(void) {
    return;
}
```

### aclspv_wrkgroup_size_id

> [!NOTE]  
> 이 속성은 SPIR-V 1.2부터 사용할 수 있습니다.

<!-- -->

> `aclspv_wrkgroup_size_id(x : u32 | id, y: u32 | id, z: u32 | id)`

- 리터럴뿐 아니라 특수 상수 ID로도 진입점의 워크그룹 크기를 지정합니다.
- 리터럴인지 특수 상수 ID인지 나타내기 위해 접두사를 사용합니다.
- 지정된 진입점에 실행 모드를 추가합니다. 다른 실행 모드를 지정하려면 [실행 모드](#aclspv_execmode)를 참조하세요.

```c
/**
 * &는 특수 상수 ID를 나타냅니다.
 * #은 리터럴을 나타냅니다.
 * 
 * 따라서 아래 선언은 크기가 다음과 같음을 나타냅니다.
 * x: ID 1로 지정된 상수
 * y: 3
 * z: ID 2로 지정된 상수
 */
__attribute__((annotate("aclspv_wrkgroup_size_id(&1, #3, &2)")))
__attribute__((annotate("aclspv_execmodel(5)")))            /** GLCompute 실행 모델 */
void entry_point_glcompute(void) {
    return;
}
```

### aclspv_location

> `aclspv_location(location : u32)`

<!-- -->

> [!WARNING]  
> [aclspv_bltin](#aclspv_bltin)과 함께 지정할 수 없습니다.

- 진입점의 인수를 선언할 때 사용합니다.
- `input(1)` 또는 `output(3)`의 위치를 지정합니다.
- 이 속성과 [aclspv_bltin](#aclspv_bltin)을 모두 지정하지 않으면 모든 입력 및 출력의 위치는 0이 됩니다.
- [aclspv_bltin](#aclspv_bltin)을 지정하면 위치는 지정되지 않습니다.

```c
__attribute__((annotate("aclspv_execmodel(0)")))
void entry_point_vertex(
    /*** 
     * 생략됨
     * 위치: 0
     * */
    __attribute__((annotate("aclspv_storage_class(1)"))) int* in_int,

    /** 
     * 위치: 1
     **/
    __attribute__((annotate("aclspv_location(1)")))
    __attribute__((annotate("aclspv_storage_class(3)"))) int* out_int
) { *out_int = *in_int; }
```

### aclspv_btcast

> `aclspv_btcast`

- 산술 형 변환이 수행될 때 변수의 최종 변환을 비트 캐스팅으로 처리하도록 지정합니다.

```c
void foo(void) {
    /** 3.20000005 */
    __attribute__((annotate("aclspv_btcast"))) float bar = 1078774989;
}
```

### aclspv_bratio

> `aclspv_bratio(weight_true_branch : u32, weight_false_branch : u32)`  

- 분기 조건이 참 또는 거짓이 될 가능성의 비율을 지정합니다.
- 이 속성은 두 갈래로 분기되는 논리 기반 제어문에 영향을 주는 성능 힌트입니다.
- 대상 제어문은 다음과 같습니다.
  - `while`
  - `if`

```c
typedef int i32;
struct foo { i32 m; };

i32 foo_read_32(struct foo* h_whatever) {
    if(h_whatever) {    /** 가중치 7 */
        __attribute__(("aclspv_bratio(7, 1)")) typedef int dummy_t;
        return h_whatever->m;
    } else {            /** 가중치 1 */
        return 0;
    }
}
```

### aclspv_layout

> `aclspv_layout(layout_number: u32)`  

- 변수 또는 타입이 따를 메모리 레이아웃을 지정합니다.
- 레이아웃 목록은 `./lib/aclty/layout.h`를 참조하세요.
- 이 속성을 지정하지 않으면 기본 레이아웃을 사용합니다. (`ACLSPV_LAYOUT_` 참조)

```c
__attribute__((annotate("aclspv_layout(1)")))
typedef struct { unsigned m_uint; } std140_uint_t;
```

### aclspv_bltin

> `aclspv_bltin(bltin_number: u32)`

<!-- -->

> [!WARNING]  
> 이 속성은 실행 모델이나 기능(capability)을 확인하거나 자동으로 해결하지 않습니다.  
> 표준 매크로를 사용해 해결하거나, 직접 선택한 값이 적절한지 확인해야 합니다.

<!-- -->

> [!WARNING]  
> [aclspv_location](#aclspv_location)과 함께 지정할 수 없습니다.

- 인터페이스 변수(입력 또는 출력)를 내장 변수로 지정합니다.
- 자세한 내용은 <https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#BuiltIn> 을 참조하세요.

```c
typedef float f32;
typedef f32 __attribute__((ext_vector_type(4))) f32x4;

__attribute__((annotate("aclspv_execmodel(0)")))
void t12_entry_point_vertex(
    /***
     * 정점 셰이더 내장 변수: 위치 출력
     * SPIR-V 내장 항목(BuiltIn): Position (0)
     * */
    __attribute__((annotate("aclspv_storage_class(3)")))
    __attribute__((annotate("aclspv_bltin(0)")))
    f32x4* gl_Position,

    /***
     * 정점 셰이더 내장 변수: 정점 인덱스 입력
     * SPIR-V 내장 항목(BuiltIn): VertexIndex (42)
     * */
    __attribute__((annotate("aclspv_storage_class(1)")))
    __attribute__((annotate("aclspv_bltin(42)")))
    int* gl_VertexIndex
) { return; }
```

## Restriction

제한 사항

> 일부 문장이나 확장은 현재 지원할 계획이 없을 수 있습니다.

- `VK_KHR_relaxed_block_layout` 동작(자유 정렬)
- POD(Plain Old Data)가 아닌 타입
- `OpenCL` 메모리 모델
- 함수 매개변수를 가리는 함수 범위 변수
- 지정 초기화자 목록
- 비물리 주소 지정 모델에서의 포인터 연산
- 전역 변수
- 입력 또는 출력 버퍼의 구조체 포인터
