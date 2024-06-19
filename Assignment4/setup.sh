#!/bin/bash

# Compilazione del passo di fusione
# Controllare di avere $ROOT prima di eseguire lo script!


cd $ROOT/BUILD
make -j 20 opt 
make -j 20 install 
cd $ROOT
bash run_case.sh

