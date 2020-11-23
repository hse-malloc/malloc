# malloc

## Requirements

* [`clang++`](https://clang.llvm.org)
* [`libc++`](https://libcxx.llvm.org/docs/UsingLibcxx.html)
* [`make`](https://www.gnu.org/software/make)
* [`cmake`](https://cmake.org)


## Build

```sh
$ git clone https://github.com/hse-malloc/malloc.git && cd malloc

$ cmake \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release \
# -DHSE_MALLOC_NO_RANDOM=TRUE # disable randomization
  -B build

$ cmake --build build
```

## Install

```sh
$ cmake --install build
```

## Test

### Simple testing

```sh
$ cd build
$ ctest --output-on-failure
```

## Docker

```sh
$ docker build -t malloc .

$ docker run --rm malloc
```
