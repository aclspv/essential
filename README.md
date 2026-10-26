```
             _                                            _   _       _ 
   __ _  ___| |___ _ ____   ___ _ ___  ___ ___  ___ _ __ | |_(_) __ _| |
  / _` |/ __| / __| '_ \ \ / (_|_) _ \/ __/ __|/ _ \ '_ \| __| |/ _` | |
 | (_| | (__| \__ \ |_) \ V / _ |  __/\__ \__ \  __/ | | | |_| | (_| | |
  \__,_|\___|_|___/ .__/ \_/ (_|_)___||___/___/\___|_| |_|\__|_|\__,_|_|
                  |_|
```


> [!WARNING]
> it is incomplete project. please keep your expectation of this application low.  

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
- libclang >= 17

## license
- [this project](./LICENSE)
- [llvm related programmes](./LICENSE-llvm)
