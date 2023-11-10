#!/bin/bash

mkdir -p build
cd build
cmake ..
# cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..

make
cd ..
