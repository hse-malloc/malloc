# malloc

> [Randomized](#randomization) implementation of standard C and C++ library dynamic memory management functions:
> * [`std::malloc`](https://en.cppreference.com/w/cpp/memory/c/malloc)
> * [`std::calloc`](https://en.cppreference.com/w/cpp/memory/c/calloc)
> * [`std::realloc`](https://en.cppreference.com/w/cpp/memory/c/realloc)
> * [`std::aligned_alloc`](https://en.cppreference.com/w/cpp/memory/c/aligned_alloc)
> * [`std::free`](https://en.cppreference.com/w/cpp/memory/c/free)
> * [`operator new`](https://en.cppreference.com/w/cpp/memory/new/operator_new)
> * [`operator delete`](https://en.cppreference.com/w/cpp/memory/new/operator_delete)
> 
> See usage examples in [examples](examples) directory.

## Build & Install

### Requirements

* [`clang++ >= 10.0.0`](https://clang.llvm.org)
* [`libc++ >= 11.0.0`](https://libcxx.llvm.org/docs/UsingLibcxx.html)
* [`cmake >= 3.18`](https://cmake.org)
* [`make`](https://www.gnu.org/software/make)

### Clone

```sh
$ git clone https://github.com/hse-malloc/malloc.git
$ cd malloc
```

### Generate

```sh
$ cmake \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release \
  -B build
```

#### Randomization

By default, address randomization is **enabled**.  
You can disable it by passing following argument while [generation](#generate):

```sh
-DHSE_MALLOC_NO_RANDOM=TRUE
```

### Build

```sh
$ cmake --build build
```

### Install

```sh
$ cmake --install build
```

## Test

> [Regenerate](#generate) with following:
> * Enable `Debug` build mode:
>   ```sh
>   -DCMAKE_BUILD_TYPE=Debug
>   ```
> * [Pass](https://libcxx.llvm.org/docs/UsingLibcxx.html) [`libc++ >= 11.0.0`](https://libcxx.llvm.org/docs/UsingLibcxx.html) headers and library location:
>   ```sh
>   -DCMAKE_CXX_FLAGS="-I<libcxx-install-prefix>/include/c++/v1" \
>   -DCMAKE_EXE_LINKER_FLAGS="-L<libcxx-install-prefix>/lib -Wl,-rpath,<libcxx-install-prefix>/lib"
>   ```

```sh
$ cd build
$ ctest --output-on-failure
```

### Docker

Available targets:
* `test`: tests (default)
* `example-c`: usage example in C using [`malloc`](https://en.cppreference.com/w/c/memory/malloc)
* `example-cpp`: usage example in C++ using [`std::malloc`](https://en.cppreference.com/w/cpp/memory/c/malloc)
* `example-cpp-new`: usage example in C++ using [`new`](https://en.cppreference.com/w/cpp/memory/new/operator_new) and [`delete`](https://en.cppreference.com/w/cpp/memory/new/operator_delete) operators

```sh
$ docker build --target <TARGET> -t malloc_<TARGET> .

$ docker run --rm malloc_<TARGET>
```
