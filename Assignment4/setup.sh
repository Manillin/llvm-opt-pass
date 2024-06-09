#!/bin/bash

cd /Users/chris/LLVM/BUILD
make -j 20 opt 
make -j 20 install 
cd $ROOT
bash run_case.sh

