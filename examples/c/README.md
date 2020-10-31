# example

## Preparation
[Install](https://github.com/hse-malloc/malloc#install) malloc library

## Build & Run

```sh
$ cmake \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_CLANG_TIDY="clang-tidy" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -B build
$ cmake --build build
$ ./build/malloc_example_c
```

### Docker

```sh
# build image with library
$ docker build -t malloc ../lib

# build example
$ docker build -t malloc_example_c .

# run example
$ docker run --rm malloc_example_c
```
