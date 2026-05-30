# aclspv
> it is incomplete project. please keep your expectation of this application low.  

- C-family language with annotation attributes to SPIR-V compiler library.  
- Uses llvm-c, clang-c to avoid memory corruption caused by static fiasco 
- `lib` for library source to be compiled
- `inc` for library header
- This project compiles the dialect of OpenCL C. For further, see [here](./ACL.md)

## How to build
```sh
cmake -S. -Bbuild
cmake --build build
```

## language
- ISO C90 with strict rule.

## dependency
- cmake >= 3.10
- c compiler which compiles c90+
-


## license
- [this project](./LICENSE)
- [llvm related programmes](./LICENSE-llvm)
