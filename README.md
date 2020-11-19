# malloc

## Requirements

* `clang`
* `make`
* `cmake`


## Build

```sh
$ git clone https://github.com/hse-malloc/malloc.git && cd malloc

$ cmake \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_CLANG_TIDY="clang-tidy" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -B build

$ cmake --build build
```

It is possible to disable randomization by setting the `-DHSE_MALLOC_NO_RANDOM=TRUE` 

## Install

```sh
$ cmake --install build
```

## Test

### Simple testing

```sh
$ cd build
$ ctest
```

### Coverage analysis

```sh
$ cd build/test
$ ./malloc_test 1>/dev/null 2>/dev/null && \
 llvm-profdata merge -o output.profdata default.profraw && \
 llvm-cov report malloc_test --instr-profile=output.profdata && \
 llvm-cov show malloc_test --instr-profile=output.profdata -format=html > report.html
```

## Docker

```sh
$ docker build -t malloc .

$ docker run --rm malloc
```
