# malloc

## Requirements

* `gcc`
* `make`
* `cmake`


## Build

```sh
$ git clone https://github.com/hse-malloc/malloc.git && cd malloc

$ cmake -S . -B build

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
