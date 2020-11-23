# malloc

## Requirements

* [`clang++ >= 10.0.0`](https://clang.llvm.org)
* [`libc++ >= 11.0.0`](https://libcxx.llvm.org/docs/UsingLibcxx.html)
* [`cmake >= 3.18`](https://cmake.org)
* [`make`](https://www.gnu.org/software/make)


## Generate

```sh
$ git clone https://github.com/hse-malloc/malloc.git && cd malloc

$ cmake \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release \
# -DHSE_MALLOC_NO_RANDOM=TRUE # disable randomization
  -B build
```
## Build

```sh
$ cmake --build build
```

## Install

```sh
$ cmake --install build
```

## Test

> [Regenerate](#generate) with following:
> * Enable `Debug` build mode:
>   ```sh
>   -DCMAKE_BUILD_TYPE=Debug
>   ```
> * Pass [`libc++ >= 11.0.0`](https://libcxx.llvm.org/docs/UsingLibcxx.html) headers and library location:
>   ```sh
>   -DCMAKE_CXX_FLAGS="-I<libcxx-install-prefix>/include/c++/v1" \
>   -DCMAKE_EXE_LINKER_FLAGS="-L<libcxx-install-prefix>/lib -Wl,-rpath,<libcxx-install-prefix>/lib"
>   ```

```sh
$ cd build
$ ctest --output-on-failure
```

### Usage

See examples in [examples](examples) folder.

## Docker

```sh
$ docker build -t malloc .

$ docker run --rm malloc
```
