NAME="LocalOpts"



# Header file
touch SRC/llvm-project-llvmorg-17.0.6/llvm/include/llvm/Transforms/Utils/$NAME.h
code  SRC/llvm-project-llvmorg-17.0.6/llvm/include/llvm/Transforms/Utils/$NAME.h

# Src file
touch SRC/llvm-project-llvmorg-17.0.6/llvm/lib/Transforms/Utils/$NAME.cpp
code SRC/llvm-project-llvmorg-17.0.6/llvm/lib/Transforms/Utils/$NAME.cpp

# Open usefull files
echo "1. Add $NAME (.cpp)  in CMakeLists.txt"
code SRC/llvm-project-llvmorg-17.0.6/llvm/lib/Transforms/Utils/CMakeLists.txt

echo -e "\n2. Add MODULE_PASS(\"local_opts\", $NAME()) in PassRegistry.def"
code SRC/llvm-project-llvmorg-17.0.6/llvm/lib/Passes/PassRegistry.def
echo -e "ATTENZIONE : al posto di "LOOP_PASS" può essere che vada messo FUNCTION_PASS,...\n"

echo -e "3. Add #include \"llvm/Transforms/Utils/$NAME.h\" in PassBuilder.cpp"
code SRC/llvm-project-llvmorg-17.0.6/llvm/lib/Passes/PassBuilder.cpp


