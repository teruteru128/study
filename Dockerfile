FROM ubuntu:24.04 as build

WORKDIR /usr/src/study

RUN DEBIAN_FRONTEND=noninteractive apt install ninja-build cmake build-essential autopoint libcurl4-gnutls-dev libgmp-dev libsodium-dev libupnp-dev libxmlrpc-core-c3-dev uuid-dev libcunit1-dev libjson-c-dev libtommath-dev libtomcrypt-dev mingw-w64 gettext postgresql-server-dev-14 -y

COPY . .

RUN cmake -S . -B build -G Ninja
RUN ninja -C build all

FROM ubuntu:24.04
WORKDIR /usr/src/study
RUN DEBIAN_FRONTEND=noninteractive apt install ninja-build autopoint libcurl4-gnutls libgmp libsodium libupnp libxmlrpc-core-c3 uuid libcunit1 libjson-c libtommath libtomcrypt mingw-w64 gettext -y
COPY --from=build . .
CMD ["/usr/bin/bash"]
