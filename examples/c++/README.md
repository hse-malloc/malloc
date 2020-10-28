# example

## Preparation
[Install](https://github.com/hse-malloc/malloc#install) malloc library

## Build & Run

```sh
$ cmake -S . -B build
$ cmake --build build
$ ./build/malloc_example_c++
```

### Docker

```sh
# build image with library
$ docker build -t malloc ../lib

# build example
$ docker build -t malloc_example_c++ .

# run example
$ docker run --rm malloc_example_c++
```
