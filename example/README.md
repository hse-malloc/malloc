# example

## Build & Run

```sh
cmake -S . -B build
cmake --build build
cma
./build/example
```

### Docker

```sh
# build image with library
$ docker build -t malloc ../lib

# build example
$ docker build -t malloc_example .

# run example
$ docker run --rm malloc_example
```
