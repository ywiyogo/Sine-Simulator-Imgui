#!/bin/bash
mkdir -p build/web
cd build/web
emcmake cmake ../..
make -j$(nproc)

