//Da come presupposto che sia già stato fatto il controllo di adiacenza)
bool areControlFlowEquivalent(Loop *L0, Loop *L1, DominatorTree &DT, PostDominatorTree &PDT){
    
    SmallVector<BasicBlock *> L0exitBlocks;
    L0->getExitBlocks(L0exitBlocks);

    for (BasicBlock* exitBlock : L0exitBlocks ) {
        BasicBlock *nextBB = exitBlock->getTerminator()->getSuccessor(0);
        int n_succ = exitBlock->getTerminator()->getNumSuccessors();
        if(n_succ > 0 && !DT.dominates(exitBlock ,nextBB ) && !PDT.dominates(nextBB, exitBlock)){
            return false;
        }   
    }

    return true;
}



bool haveSameTripCount(Loop *L0, Loop *L1, ScalarEvolution &SE){
    const SCEV *S1 = SE.getBackedgeTakenCount(L0);
    const SCEV *S2 = SE.getBackedgeTakenCount(L1);

    if (S1 == S2) {
        outs()<<"I 2 loop hanno lo stesso numero di iterazioni!\n";
        return true;
    } else {
        return false;
    }
}
