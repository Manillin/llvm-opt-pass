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

bool areLoopsAdjacent(const Loop *L1, const Loop *L2)
{
    // Controllo se i due loop sono nulli
    if (!L1 || !L2)
    {
        return false;
    }
    outs() << "\n";

    // nel preheader del loop L1 c'è un branch che va al preheader del loop L2
    if (L1->isGuarded())
    {
        outs() << "\n> L1 è guarded\n";
        BasicBlock *L1preheader = L1->getLoopPreheader();
        // prendo l'ultimo istruzione del preheader del loop L1
        Instruction *L1preheaderTerminator = L1preheader->getTerminator();
        // controllo se l'istruzione è un branch
        if (BranchInst *L1preheaderBranch =
                dyn_cast<BranchInst>(L1preheaderTerminator))
        {
            // controllo se il branch ha due operandi
            if (L1preheaderBranch->getNumSuccessors() == 2)
            {
                // prendo il secondo operando del branch
                BasicBlock *L1preheaderBranchSuccessor1 =
                    L1preheaderBranch->getSuccessor(0);
                BasicBlock *L1preheaderBranchSuccessor2 =
                    L1preheaderBranch->getSuccessor(1);

                if (L1preheaderBranchSuccessor1 == L2->getHeader() ||
                    L1preheaderBranchSuccessor2 == L2->getHeader())
                {
                    outs() << "\n> Adiacenti! \n";
                    return true;
                }
            }
        }
        return false;
    }

    // non Guarded
    if (!L1->isGuarded())
    {
        outs() << "> L1 non è guarded\n";
        SmallVector<BasicBlock *> L1exitBlocks;
        // prendo tutti gli exit block del loop e li metto in un vettore
        L1->getExitBlocks(L1exitBlocks);
        // controllo se il preheader del loop L2 è diverso all'exit block del loop
        // L1, nel caso in cui sia diverso ritorno false vado a controllare che
        // tutte le uscite convergono verso un unico punto s
        for (BasicBlock *exitingblock : L1exitBlocks)
        {
            if (exitingblock != L2->getLoopPreheader())
            {
                return false;
            }
        }
        outs() << "> Adiacenti! \n";
        return true;
    }
}

bool haveSameTripCount(Loop *L1, Loop *L2, ScalarEvolution &SE)
{
    const SCEV *S1 = SE.getBackedgeTakenCount(L1);
    const SCEV *S2 = SE.getBackedgeTakenCount(L2);

    if (S1 == S2)
    {
        outs() << "> I 2 loop hanno lo stesso numero di iterazioni!\n";
        return true;
    }
    else
    {
        return false;
    }
}

bool areControlFlowEquivalent(Loop *L1, Loop *L2, DominatorTree &DT,
                              PostDominatorTree &PDT)
{

    SmallVector<BasicBlock *> L1exitBlocks;
    L1->getExitBlocks(L1exitBlocks);

    for (BasicBlock *exitBlock : L1exitBlocks)
    {
        BasicBlock *nextBB = exitBlock->getTerminator()->getSuccessor(0);
        int n_succ = exitBlock->getTerminator()->getNumSuccessors();
        if (n_succ > 0 && !DT.dominates(exitBlock, nextBB) &&
            !PDT.dominates(nextBB, exitBlock))
        {
            return false;
        }
    }
    outs() << "> Control Flow equivalent! \n";
    return true;
}

bool hasNegativeDependencies(Loop *L1, Loop *L2, DependenceInfo &DI)
{
    // Itera attraverso tutti i basic blocks del loop L1
    for (auto *BB1 : L1->blocks())
    {
        // Itera attraverso tutte le istruzioni di BB1
        for (auto &I1 : *BB1)
        {
            // Itera attraverso tutti i basic blocks del loop L2
            for (auto *BB2 : L2->blocks())
            {
                // Itera attraverso tutte le istruzioni di BB2
                for (auto &I2 : *BB2)
                {
                    // Ottieni l'informazione di dipendenza tra le istruzioni I1 e I2
                    auto D = DI.depends(&I1, &I2, true);
                    // Se c'è una dipendenza
                    if (D)
                    {
                        // Se la dipendenza è di distanza negativa
                        // isDirectionNegative() restituisce true se la dipendenza è di
                        // distanza negativa
                        if (D->isDirectionNegative())
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    outs() << "> Nessuna dipendenza di distanza negativa trovata! \n";
    return false; // Nessuna dipendenza di distanza negativa trovata
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
    outs() << "\n";
    outs() << "\nStart loop fusion opt...\n";

    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    // L1 -> previousLoop ; L2 -> currentLoop
    Loop *L1 = nullptr;
    bool modified = false;

    for (auto lit = LI.rbegin(); lit != LI.rend(); ++lit)
    {
        Loop *L2 = *lit;

        if (L1)
        {
            if (areLoopsAdjacent(L1, L2) && haveSameTripCount(L1, L2, SE) &&
                areControlFlowEquivalent(L1, L2, DT, PDT) &&
                !hasNegativeDependencies(L1, L2, DI) && L1->isLoopSimplifyForm() &&
                L2->isLoopSimplifyForm())
            {
                outs() << "Trovati loop adiacenti candidati per la fusione! \n";
                fuseLoop(L1, L2);
                outs() << "\n---------- Fusione dei loop completata con successo"
                          "----------\n\n";
                modified = true;
                // Salta il prossimo loop in quanto è già stato fuso
                L2 = *lit;
                L1 = L2;
                continue;
            }
        }

        L1 = L2;
    }

    outs() << "\nend of loop fusion opt...\n";

    if (modified)
        return PreservedAnalyses::none();
    else
        return PreservedAnalyses::all();
}