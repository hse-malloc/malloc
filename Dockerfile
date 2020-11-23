FROM ubuntu:20.04 AS build-env

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
  && apt-get install --yes --no-install-recommends \
    apt-transport-https \
    ca-certificates \
    git \
    gnupg \
    software-properties-common \
    wget \
  && wget -O - "https://apt.kitware.com/keys/kitware-archive-latest.asc" 2> /dev/null \
  | gpg --dearmor - \
  | apt-key add - \
  && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main' \
  && wget -O - "https://apt.llvm.org/llvm-snapshot.gpg.key" 2> /dev/null \
  | apt-key add - \
  && add-apt-repository "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-11 main" \
  && apt-get update \
  && apt-get install --yes --no-install-recommends \
    cmake \
    make \
    libc++-11-dev \
    libc++1-11 \
    libc++abi-11-dev \
    libc++abi1-11 \
    clang-11 \
    clang-tools-11 \
    clang-tidy-11 \
    clang-format-11 \
    lldb-11 \
    lld-11 \
  && rm -rf /var/lib/apt/lists/*


FROM build-env AS build

WORKDIR /root/malloc/

COPY . .

RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER="clang++-11" \
    -DCMAKE_CXX_CLANG_TIDY="clang-tidy-11" \
    -DCMAKE_CXX_FLAGS="-I/lib/llvm-11/include/c++/v1" \
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/llvm-11/lib -Wl,-rpath,/lib/llvm-11/lib -v" \
    -B build \
  && cmake --build build -v \
  && cmake --install build


FROM build AS test

WORKDIR build

ENTRYPOINT ["ctest", "--output-on-failure"]


FROM build-env AS malloc-installed

COPY --from=build /usr/local/lib/lib*malloc.a /usr/local/lib/
COPY --from=build /usr/local/include/malloc/* /usr/local/include/malloc/
COPY --from=build /usr/local/lib/cmake/malloc/* /usr/local/lib/cmake/malloc/


FROM malloc-installed AS example-c

WORKDIR /root/malloc/examples/c

COPY examples/c .

RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="clang-11" \
    -DCMAKE_CXX_COMPILER="clang++-11" \
    -DCMAKE_CXX_CLANG_TIDY="clang-tidy-11" \
    -DCMAKE_CXX_FLAGS="-I/lib/llvm-11/include/c++/v1" \
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/llvm-11/lib -Wl,-rpath,/lib/llvm-11/lib -v" \
    -B build \
  && cmake --build build -v 

CMD ["build/malloc_example_c"]


FROM malloc-installed AS example-cpp

WORKDIR /root/malloc/examples/cpp/malloc

COPY examples/cpp/malloc .

RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER="clang++-11" \
    -DCMAKE_CXX_CLANG_TIDY="clang-tidy-11" \
    -DCMAKE_CXX_FLAGS="-I/lib/llvm-11/include/c++/v1" \
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/llvm-11/lib -Wl,-rpath,/lib/llvm-11/lib -v" \
    -B build \
  && cmake --build build -v 

CMD ["build/malloc_example_cpp"]


FROM malloc-installed AS example-cpp-new

WORKDIR /root/malloc/examples/cpp/new

COPY examples/cpp/new .

RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER="clang++-11" \
    -DCMAKE_CXX_CLANG_TIDY="clang-tidy-11" \
    -DCMAKE_CXX_FLAGS="-I/lib/llvm-11/include/c++/v1" \
    -DCMAKE_EXE_LINKER_FLAGS="-L/usr/llvm-11/lib -Wl,-rpath,/lib/llvm-11/lib -v" \
    -B build \
  && cmake --build build -v 

CMD ["build/malloc_example_cpp_new"]

FROM test
