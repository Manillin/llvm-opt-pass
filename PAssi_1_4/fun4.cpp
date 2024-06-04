/* Punto 4 : non ci possono essere dipendenze con distanza negativa
Una dipendenza con distanza negativa è una dipendenza che va da un'iterazione successiva a un'iterazione precedente.

Devi aggiungere la riga di codice DependenceInfo &DI = AM.getResult<DependencyAnalysis>(F); e includere il file llvm/Analysis/DependenceAnalysis.h
DependenceInfo è utilizzato in un pass LLVM, permette al pass di accedere alle informazioni sulle dipendenze delle istruzioni all'interno della funzione F
*/
/**
 * Controllo se ci sono dipendenze con distanza negativa tra due loop.
 * @param L1 Primo loop da controllare.
 * @param L2 Secondo loop da controllare.
 * @param DI Informazioni sulle dipendenze tra le istruzioni.
 * @return Vero se c'è una dipendenza con distanza negativa tra i due loop, falso altrimenti.
*/

bool hasNegative_dep(Loop *L1, Loop *L2, DependenceInfo &DI) {
    // Itera attraverso tutti i basic blocks del loop L1
    for (auto *BB1 : L1->blocks()) {
        // Itera attraverso tutte le istruzioni di BB1
        for (auto &I1 : *BB1) {
            // Itera attraverso tutti i basic blocks del loop L2
            for (auto *BB2 : L2->blocks()) {
                // Itera attraverso tutte le istruzioni di BB2
                for (auto &I2 : *BB2) {
                    // Ottieni l'informazione di dipendenza tra le istruzioni I1 e I2
                    auto D = DI.depends(&I1, &I2, true);
                    // Se c'è una dipendenza
                    if (D) {
                        // Se la dipendenza è di distanza negativa
                        //isDirectionNegative() restituisce true se la dipendenza è di distanza negativa  
                        if(D -> isDirectionNegative()){
                            return true;
                        }

                    }
                }
            }
        }
    }
    return false; // Nessuna dipendenza di distanza negativa trovata
}
