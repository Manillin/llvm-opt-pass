# Creazione del CFG del codice IR prima e dopo la fusione dei loop e applicazione del passo di fusione

# Assicurarsi di avere $ROOT come root directory di llvm 

cd $ROOT

FILENAME="loopFusion"

ls TEST/$FILENAME.ll > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "File not found"
    exit 1
fi


echo -e "\n------- Nome del FILE : ${FILENAME}.ll -------\n"

# CFG del codice IR prima della fusione
INSTALL/bin/opt -p=dot-cfg "TEST/${FILENAME}.ll" > /dev/null 2>&1
mv \.execute2loop.dot ./initialCFG.dot
dot -Tpng initialCFG.dot -o iCFG.png

echo -e "\n------- CFG iniziale creato: iCFG.png -------\n"

echo -e "\n\n\n<<<<<<<<<<< ESECUZIONE PASSO DI FUSIONE >>>>>>>>>>>\n\n\n"

# Applicazione del passo di fusion
INSTALL/bin/opt -p=loopfusionpass "TEST/${FILENAME}.ll" -o TEST/LFbash.bc 
INSTALL/bin/llvm-dis TEST/LFbash.bc -o TEST/wLF.ll
rm TEST/LFbash.bc

echo -e "\n\n\n<<<<<<<<<<< PASSO DI FUSIONE APPLICATO >>>>>>>>>>>\n\n\n"

# CFG del codice IR dopo la fusione
INSTALL/bin/opt -p=dot-cfg TEST/wLF.ll > /dev/null 2>&1
mv \.execute2loop.dot ./wLF.dot
dot -Tpng wLF.dot -o fusion.png

echo -e "\n------- CFG finale creato: fusion.png -------\n"


# Cleanup dei file temporanei
rm wLF.dot
rm initialCFG.dot