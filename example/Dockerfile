FROM malloc AS build

WORKDIR /root/example

COPY . .

RUN cmake -S . -B build \
  && cmake --build build

ENTRYPOINT ["./build/malloc_example"]
