FROM ubuntu:20.04 AS build-env

RUN apt-get update \
  && apt-get install --yes --no-install-recommends \
    apt-transport-https \
    ca-certificates \
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
    -B build \
  && cmake --build build \
  && cmake --install build


FROM build AS test

WORKDIR build

ENTRYPOINT ["ctest"]
