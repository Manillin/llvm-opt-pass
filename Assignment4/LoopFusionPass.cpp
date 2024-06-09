#include "llvm/Transforms/Utils/LoopFusionPass.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/IR/Constants.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using namespace llvm;

bool areLoopsAdjacent(Loop *L1, Loop *L2) { return true; }

bool haveSameTripCount(Loop *L1, Loop *L2, ScalarEvolution &SE) { return true; }

bool areControlFlowEquivalent(Loop *L1, Loop *L2, DominatorTree &DT,
                              PostDominatorTree &PDT)
{
    return true;
}

bool noNegativeDistanceDependencies(Loop *L1, Loop *L2, DependenceInfo &DI)
{
    return true;
}

void replaceInductionVariables(Loop *L1, Loop *L2)
{

    // Ottieni le variabili di induzione per entrambi i loop
    PHINode *indVar1 = L1->getCanonicalInductionVariable();
    PHINode *indVar2 = L2->getCanonicalInductionVariable();

    // Controlla se entrambe le variabili di induzione esistono
    if (!indVar1 || !indVar2)
    {
        outs() << "Warning: Uno dei loop non ha variabile di induzione.\n";
        return;
    }

    // Sostituisci gli usi della variabile di induzione del loop2 con quelli del
    // loop1
    std::vector<Instruction *> users;
    for (auto &Use : indVar2->uses())
    {
        Instruction *User = cast<Instruction>(Use.getUser());
        if (L2->contains(User))
        {
            users.push_back(User);
        }
    }

    for (auto *User : users)
    {
        User->replaceUsesOfWith(indVar2, indVar1);
    }

    // Verifica se la sostituzione è avvenuta correttamente
    bool replacementSuccess = true;
    for (auto &Use : indVar2->uses())
    {
        Instruction *User = cast<Instruction>(Use.getUser());
        if (L2->contains(User) && User->getOperand(0) == indVar2)
        {
            replacementSuccess = false;
            break;
        }
    }

    if (replacementSuccess)
    {
        outs() << "\n---------- Variabili di induzione sostituite con successo "
                  "----------\n\n";
    }
    else
    {
        outs() << "\n\n\n\n\nWARNING: Errore nella sostituzione delle variabili di "
                  "induzione.\n\n\n\n\n";
    }
}

Loop *fuseLoop(Loop *L1, Loop *L2)
{
    outs() << "Invocato fuse loop definitivo\n";

    // replace the induction variable of the second loop with the one of the first
    replaceInductionVariables(L1, L2);

    // get the basic blocks of interest necessary for the fusion
    BasicBlock *header1 = L1->getHeader();
    BasicBlock *latch1 = L1->getLoopLatch();
    BasicBlock *body1 = latch1->getSinglePredecessor();
    BasicBlock *exit1 = L1->getExitBlock();

    BasicBlock *header2 = L2->getHeader();
    BasicBlock *preheader2 = L2->getLoopPreheader();
    BasicBlock *latch2 = L2->getLoopLatch();
    BasicBlock *body2 = latch2->getSinglePredecessor();
    BasicBlock *exit2 = L2->getExitBlock();

    // controlliamo quale dei successori del header è il body del loop
    BasicBlock *body2entry;
    if (L2->contains(header2->getTerminator()->getSuccessor(0)))
    {
        // successore 0 del branch del header è il body
        body2entry = header2->getTerminator()->getSuccessor(0);
    }
    else
    {
        // successore 1 del branch del header è il body
        body2entry = header2->getTerminator()->getSuccessor(1);
    }

    // controlla se preheader2 è uguale all'exit block 1
    assert(preheader2 == exit1 && "preheader2 is not equal to exit2");

    // cambia il successor del header1 (caso terminazione loop) a exit block di L2
    header1->getTerminator()->replaceSuccessorWith(preheader2, exit2);
    // cambia il successor del body1 a body2
    body1->getTerminator()->replaceSuccessorWith(latch1, body2entry);
    // TODO: check
    // cambia successor del header2 a latch2
    ReplaceInstWithInst(header2->getTerminator(), BranchInst::Create(latch2));

    // cambia il successor del body2 a latch1 per chiudere il loop
    body2->getTerminator()->replaceSuccessorWith(latch2, latch1);

    // Il loop fuso si troverà in L1
    return L1;
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

    Loop *prevLoop = nullptr;
    bool modified = false;

    for (auto lit = LI.rbegin(); lit != LI.rend(); ++lit)
    {
        Loop *currLoop = *lit;

        if (prevLoop)
        {
            if (areLoopsAdjacent(prevLoop, currLoop) &&
                haveSameTripCount(prevLoop, currLoop, SE) &&
                areControlFlowEquivalent(prevLoop, currLoop, DT, PDT) &&
                noNegativeDistanceDependencies(prevLoop, currLoop, DI) &&
                prevLoop->isLoopSimplifyForm() && currLoop->isLoopSimplifyForm())
            {
                outs() << "Trovati loop adiacenti candidati per la fusione! \n";
                fuseLoop(prevLoop, currLoop);
                // fuseLoops2(F, AM, prevLoop, currLoop);
                //  fuseLoop(prevLoop, currLoop);
                outs() << "\n---------- Fusione dei loop completata con successo"
                          "----------\n\n";
                modified = true;
                // Salta il prossimo loop in quanto è già stato fuso
                currLoop = *lit;
                prevLoop = currLoop;
                continue;
            }
        }

        prevLoop = currLoop;
    }

    outs() << "\nEnd of loop fusion opt...\n";

    if (modified)
        return PreservedAnalyses::none();
    else
        return PreservedAnalyses::all();
}
