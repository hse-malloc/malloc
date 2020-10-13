FROM gcc:latest AS build-env

ARG CMAKE_VERSION=3.18.4

RUN curl --fail --show-error --silent --location --output /tmp/cmake-install.sh \
    "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh" \
  && chmod u+x /tmp/cmake-install.sh \
  && /tmp/cmake-install.sh --prefix=/usr/local --skip-license \
  && rm /tmp/cmake-install.sh


FROM build-env AS build

WORKDIR /root/malloc/

COPY . .

RUN cmake -S . -B build \
  && cmake --build build \
  && cmake --install build


FROM build AS test

WORKDIR build

ENTRYPOINT ["ctest"]
