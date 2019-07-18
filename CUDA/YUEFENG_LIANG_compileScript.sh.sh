#!/bin/bash

# script to compile && run in batch

FILES=( \
cuda.cu
)

for k in ${FILES[*]}; do
  echo \*compiling $k\*
  nvcc -Wno-deprecated-gpu-targets -Xcompiler -fopenmp ${k} && time ./a.out 10000000 || error compiling $k
  echo
done