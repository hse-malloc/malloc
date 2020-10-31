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
  -DCMAKE_CXX_CLANG_TIDY="clang-tidy" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -B build

$ cmake --build build
```

## Install

```sh
$ cmake --install build
```

## Test

```sh
$ cd build
$ ctest
```

## Docker

```sh
$ docker build -t malloc .

$ docker run --rm malloc
```
