> [!WARNING]
> it is incomplete project. please keep your expectation of this application low.  

# aclspv

- this project is written in strict iso c90 to make this project portable as possible in my knowledge.
- this project could be compiled in higher version or dialect of c to increase its performance.

- c-family language with annotation attributes to SPIR-V compiler library.  
- uses llvm-c, clang-c to avoid memory corruption caused by static fiasco 
- `lib` for library source to be compiled
- `inc` for library header
- `src` is for dsl of generated headers.
- this project compiles a c-like language with attributes.
- list of attribute specs is written [here](./aclang.md)

## How to build
```sh
cmake -S. -Bbuild
cmake --build build
```

## dependency
- cmake >= 3.10
- c compiler which compiles iso c90 or later which supports compile definition (-D)
- libclang

## license
- [this project](./LICENSE)
- [llvm related programmes](./LICENSE-llvm)
