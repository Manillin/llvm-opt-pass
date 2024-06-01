/**
 * Controllo se i due loop sono adiacenti.
 *
 * @param L0 Primo loop da controllare.
 * @param L1 Secondo loop da controllare.
 * @return Vero se i due loop sono adiacenti, falso altrimenti.
 */
bool are_loops_adjacent(const Loop* L0, const Loop* L1) {
    // Controllo se i due loop sono nulli
    if (!L0 || !L1) {
        return false;
    }

    //nel preheader del loop L0 c'è un branch che va al preheader del loop L1
    if (L0->isGuarded()) {
        outs()<<"L0 è guarded\n";
        BasicBlock* L0preheader = L0->getLoopPreheader();
        //prendo l'ultimo istruzione del preheader del loop L0
        Instruction* L0preheaderTerminator = L0preheader->getTerminator();
        //controllo se l'istruzione è un branch
        if (BranchInst* L0preheaderBranch = dyn_cast<BranchInst>(L0preheaderTerminator)) {
            //controllo se il branch ha due operandi
            if (L0preheaderBranch->getNumSuccessors() == 2) {
                //prendo il secondo operando del branch
                BasicBlock* L0preheaderBranchSuccessor1 = L0preheaderBranch->getSuccessor(0);
                BasicBlock* L0preheaderBranchSuccessor2 = L0preheaderBranch->getSuccessor(1);

            if (L0preheaderBranchSuccessor1 == L1->getHeader() || L0preheaderBranchSuccessor2 == L1->getHeader()) {
                return true;
            }
            }

        }
        return false;
    }

//non Guarded
    if(!L0->isGuarded()){
    outs()<<"L0 non è guarded\n";
    SmallVector<BasicBlock *> L0exitBlocks;
    //prendo tutti gli exit block del loop e li metto in un vettore
    L0->getExitBlocks(L0exitBlocks);
    //controllo se il preheader del loop L1 è diverso all'exit block del loop L0, nel caso in cui sia diverso ritorno false
    //vado a controllare che tutte le uscite convergono verso un unico punto s
    for (BasicBlock* exitingblock : L0exitBlocks) {
            if(exitingblock != L1->getLoopPreheader()){
                return false;
            }
        }
        return true;
    }
}
