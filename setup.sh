#!/bin/bash

cd $ROOT/BUILD
make opt 
make -j 10 install 
cd $ROOT
INSTALL/bin/opt -p=localopts TEST/basic_sr.ll -o basic_sr.bc
INSTALL/bin/llvm-dis basic_sr.bc -o TEST/testost.ll