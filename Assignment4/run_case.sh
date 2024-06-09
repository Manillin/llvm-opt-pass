cd $ROOT


# Creazione del CFG prima della loopfusion
INSTALL/bin/opt -p=dot-cfg TEST/test_array.ll
mv \.twoloops.dot ./initialCFG.dot
dot -Tpng initialCFG.dot -o iCFG.png


INSTALL/bin/opt -p=loopfusionpass TEST/test_array.ll -o TEST/LFbash.bc
INSTALL/bin/llvm-dis TEST/LFbash.bc -o TEST/wLF.ll
rm TEST/LFbash.bc

# Creazione del CFG dopo la loopfusion
INSTALL/bin/opt -p=dot-cfg TEST/wLF.ll
mv \.twoloops.dot ./wLF.dot
dot -Tpng wLF.dot -o fusion.png

rm wLF.dot
rm initialCFG.dot