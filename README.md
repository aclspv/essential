> [!WARNING]
> it is incomplete project. please keep your expectation of this application low.  

# aclspv

summary  
- this project is written in strict iso c90 to make this project as portable and stable as possible in my knowledge.
- this project could be compiled in higher version or dialect of c to increase its performance.
- this project compiles c-family language with annotation attributes to SPIR-V compiler library. - 
- this project is can generate flexible form of entry points.
- uses clang-c as frontend to generate ast.
- list of attribute specs is written [here](./aclang.md)

tree  
- `lib` for library source to be compiled
- `inc` for library header
- `src` is for dsl of generated headers.

## how to build
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
