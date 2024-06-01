#include "llvm/Transforms/Utils/LoopFusionPass.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include <llvm/IR/Constants.h>

using namespace llvm;

bool areLoopsAdjacent(Loop *L1, Loop *L2)
{
    // TODO
}

bool haveSameTripCount(Loop *L1, Loop *L2, ScalarEvolution &SE)
{
    // TODO
}

bool areControlFlowEquivalent(Loop *L1, Loop *L2, DominatorTree &DT,
                              PostDominatorTree &PDT)
{
    // TODO
}

bool noNegativeDistanceDependencies(Loop *L1, Loop *L2, DependenceInfo &DI)
{
    // TODO
}

void replaceInductionVariables(llvm::Loop *loop1, llvm::Loop *loop2,
                               llvm::ScalarEvolution &SE)
{
    // Ottenere le variabili di induzione per entrambi i loop
    llvm::PHINode *indVar1 = loop1->getInductionVariable(SE);
    llvm::PHINode *indVar2 = loop2->getInductionVariable(SE);

    // Controllare se entrambe le variabili di induzione esistono
    if (!indVar1 || !indVar2)
    {
        outs() << "Warning: Uno dei loop non ha variabile di induzione.\n";
        return;
    }

    // Sostituire gli usi della variabile di induzione del loop2 con quelli del
    // loop1
    indVar2->replaceAllUsesWith(indVar1);
}

void fuseLoops(Loop *L1, Loop *L2, LoopInfo &LI, DominatorTree &DT)
{
    // Ottenere i blocchi di intestazione (header) e latch per entrambi i loop
    BasicBlock *Header1 = L1->getHeader();
    BasicBlock *Latch1 = L1->getLoopLatch();
    BasicBlock *Header2 = L2->getHeader();
    BasicBlock *Latch2 = L2->getLoopLatch();
    BasicBlock *Preheader2 = L2->getLoopPreheader();

    // Rimuovere i collegamenti esistenti tra i blocchi del loop 2 e i blocchi
    // fuori dal loop
    Preheader2->getTerminator()->replaceUsesOfWith(Header2, Latch1);

    // Collegare il latch del loop 1 al preheader del loop 2
    BranchInst::Create(Preheader2, Latch1->getTerminator());
    Latch1->getTerminator()->eraseFromParent();

    // Collegare il latch del loop 2 al header del loop 1 per formare un unico
    // ciclo
    BranchInst::Create(Header1, Latch2->getTerminator());
    Latch2->getTerminator()->eraseFromParent();

    // Aggiornare il LoopInfo
    for (auto *BB : L2->blocks())
    {
        L1->addBasicBlockToLoop(BB, LI);
    }

    // Aggiornare il DominatorTree
    DT.changeImmediateDominator(Header2, Latch1);
}

PreservedAnalyses LoopFusionPass::run(Function &F,
                                      FunctionAnalysisManager &AM)
{
    outs() << "\nStart loop fusion opt...\n";

    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    std::vector<std::pair<Loop *, Loop *>> fusionCandidates;

    std::vector<Loop *> loops(LI.begin(), LI.end());
    for (size_t i = 0; i < loops.size() - 1; ++i)
    {
        Loop *L1 = loops[i];
        Loop *L2 = loops[i + 1];

        if (areLoopsAdjacent(L1, L2) && haveSameTripCount(L1, L2, SE) &&
            areControlFlowEquivalent(L1, L2, DT, PDT) &&
            noNegativeDistanceDependencies(L1, L2, DI))
        {
            fusionCandidates.push_back(std::make_pair(L1, L2));
            ++i; // Salta il prossimo loop in quanto è già candidato per la fusion
        }
    }

    for (auto &candidate : fusionCandidates)
    {
        Loop *L1 = candidate.first;
        Loop *L2 = candidate.second;

        replaceInductionVariables(L1, L2, SE);
        fuseLoops(L1, L2, LI, DT);
    }

    return PreservedAnalyses::all();
}