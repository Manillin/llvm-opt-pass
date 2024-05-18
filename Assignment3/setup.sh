#!/bin/bash
# currently debug only
cd /Users/chris/LLVM/BUILD
make -j 15 opt 
make -j 15 install 
cd $ROOT
INSTALL/bin/opt -p=loopwalk TEST/LICM.ll 

# uncomment when ready: 

# cd $ROOT
# INSTALL/bin/opt -p=loopwalk TEST/LICM.ll -o TEST/LICMbash.bc
# INSTALL/bin/llvm-dis TEST/LICMbash.bc -o TEST/LICM_TEST.ll
# rm TEST/LICMbash.bc