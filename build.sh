#!/bin/bash
test -d build && rm -frv build
mkdir -pv build \
  && env CXX=clang++ \
         CXX_LD=lld \
    meson setup \
      --backend=ninja \
      --buildtype=${1:-release} \
      build \
  && ninja -C build \
  && strip --strip-all --verbose -o dumpelf build/dumpelf \
  && rm -frv build
