FROM ubuntu:20.04

WORKDIR /usr/src/study

RUN apt update && \
    apt install ninja-build autopoint libcurl4-gnutls-dev libgmp-dev libsodium-dev libupnp-dev libxmlrpc-core-c3-dev uuid-dev libcunit1-dev libjson-c-dev libtommath-dev libtomcrypt-dev mingw-w64 gettext -y

COPY . .
