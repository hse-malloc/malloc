ARG ubuntu_version=20.04
ARG llvm_version=11.0.0
ARG cmake_version=3.18.4

FROM ubuntu:$ubuntu_version AS build-env

RUN apt-get update \
  && apt-get install --yes \
    xz-utils \
    build-essential \
    make \
    curl \
  && rm -rf /var/lib/apt/lists/*

ARG cmake_version

RUN curl --fail --show-error --silent --location --output /tmp/cmake-install.sh \
    "https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}-Linux-x86_64.sh" \
  && chmod u+x /tmp/cmake-install.sh \
  && /tmp/cmake-install.sh --prefix=/usr/local --skip-license \
  && rm /tmp/cmake-install.sh

ARG ubuntu_version
ARG llvm_version

RUN curl --fail --silent --show-error --location \
    "https://github.com/llvm/llvm-project/releases/download/llvmorg-${llvm_version}/clang+llvm-${llvm_version}-x86_64-linux-gnu-ubuntu-${ubuntu_version}.tar.xz" \
  | tar -xkJC /tmp \
  && mkdir -p /usr/local/opt \
  && mv "/tmp/clang+llvm-${llvm_version}-x86_64-linux-gnu-ubuntu-${ubuntu_version}" "/usr/local/opt/llvm"

ENV PATH="/usr/local/opt/llvm/bin:${PATH}"


FROM build-env AS build

WORKDIR /root/malloc/

COPY . .

RUN cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_CLANG_TIDY="clang-tidy" \
    -B build \
  && cmake --build build \
  && cmake --install build

FROM build AS test

WORKDIR build

ENTRYPOINT ["ctest"]
