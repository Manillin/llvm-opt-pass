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
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include <llvm/IR/Constants.h>

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

void replaceInductionVariables2(llvm::Loop *loop1, llvm::Loop *loop2,
                                llvm::ScalarEvolution &SE)
{

    // Ottenere le variabili di induzione per entrambi i loop
    llvm::PHINode *indVar1 = loop1->getCanonicalInductionVariable();
    llvm::PHINode *indVar2 = loop2->getCanonicalInductionVariable();

    // Controllare se entrambe le variabili di induzione esistono
    if (!indVar1 || !indVar2)
    {
        outs() << "Warning: Uno dei loop non ha variabile di induzione.\n";
        return;
    }

    // Sostituire gli usi della variabile di induzione del loop2 con quelli del
    // loop1
    indVar2->replaceAllUsesWith(indVar1);
    outs() << "----------------------------------- ";
    outs() << "\nvariabili di induzione sostituite! \n";
}

void replaceInductionVariables(Loop *L1, Loop *L2)
{

    // Ottieni le variabili di induzione per entrambi i loop
    PHINode *indVar1 = L1->getCanonicalInductionVariable();
    PHINode *indVar2 = L2->getCanonicalInductionVariable();

    // TODO: delete when bugfree
    //   outs() << "\n---------- Variabili di induzione dei loop: ---------- \n";
    //   outs() << " IndVar1: " << *indVar1 << "\n";
    //   outs() << " IndVar2: " << *indVar2 << "\n";

    // Controlla se entrambe le variabili di induzione esistono
    if (!indVar1 || !indVar2)
    {
        outs() << "Warning: Uno dei loop non ha variabile di induzione.\n";
        return;
    }

    // TODO: delete when bugfree
    //   outs() << "\n---------- Usi della variabile di induzione che devo
    //   cambiare: "
    //             "---------- \n";
    //   for (auto &Use : indVar2->uses()) {
    //     Instruction *User = cast<Instruction>(Use.getUser());
    //     outs() << "User: " << *User << "\n";
    //   }

    // Sostituisci gli usi della variabile di induzione del loop2 con quelli del
    // loop1
    outs()
        << "\n++++++++++ Sostituzione delle variabili di induzione: ++++++++++\n";
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

    // TODO: delete when bugfree
    //   outs() << "\n---------- Stampa usi di instVar1 in L2 ----------\n";
    //   for (auto &Use : indVar1->uses()) {
    //     Instruction *User = cast<Instruction>(Use.getUser());
    //     if (L2->contains(User)) {
    //       outs() << "User: " << *User << "\n";
    //     }
    //   }

    if (replacementSuccess)
    {
        outs() << "\n---------- Variabili di induzione sostituite con successo "
                  "----------\n";
    }
    else
    {
        outs() << "\n\n\n\n\nWARNING: Errore nella sostituzione delle variabili di "
                  "induzione.\n\n\n\n\n";
    }
}

void fuseLoops_BK(Loop *L1, Loop *L2, LoopInfo &LI, DominatorTree &DT)
{
    BasicBlock *latch1 = L1->getLoopLatch();
    BasicBlock *latch2 = L2->getLoopLatch();
    BasicBlock *header1 = L1->getHeader();
    BasicBlock *header2 = L2->getHeader();
    BasicBlock *preheader1 = L1->getLoopPreheader();
    BasicBlock *preheader2 = L2->getLoopPreheader();

    if (!latch1 || !latch2 || !header1 || !header2 || !preheader1 ||
        !preheader2)
    {
        outs() << "Uno dei loop manca di un blocco essenziale.\n";
        return;
    }

    // Sostituisci i branch del latch1 e latch2 per la fusione
    Instruction *inst_latch1 = latch1->getTerminator();
    if (!inst_latch1)
    {
        outs() << "Errore: Il latch1 non ha una singola istruzione terminale.\n";
    }
    inst_latch1->replaceUsesOfWith(header1, header2);

    Instruction *inst_latch2 = latch2->getTerminator();
    if (!inst_latch2)
    {
        outs() << "Errore: Il latch2 non ha una singola istruzione terminale.\n";
    }
    inst_latch2->replaceUsesOfWith(header2, header1);

    // Nota: Il seguente passaggio sposta il blocchi del body2 dopo il latch1
    // Importante e fondamentale per fare Loop Fusion (else caso: loop unrolling)

    SmallVector<BasicBlock *, 8> body2Blocks(L2->getBlocks());
    for (BasicBlock *BB : body2Blocks)
    {
        BB->moveAfter(latch1);
    }

    // Aggiorna LoopInfo
    for (BasicBlock *BB : body2Blocks)
    {
        LI.changeLoopFor(BB, L1);
    }
    LI.changeLoopFor(latch2, L1);
    LI.erase(L2);
    outs() << "\n\nAggiornamento LoopInfo completato.\n";

    // TODO:
    // aggiornamento del dominator tree

    outs() << "Tentativo di aggiornamento del dominator tree...\n";

    // Aggiorna DominatorTree
    DT.eraseNode(header2);
    outs() << "Nodo header2 rimosso dal dominator tree.\n";
    DT.eraseNode(latch2);
    outs() << "Nodi eliminati dal dominator tree.\n";
}

void fuseLoops(Loop *L1, Loop *L2, LoopInfo &LI, DominatorTree &DT)
{

    outs() << "\n++++++++++ Inizio fusione dei loop: ++++++++++\n";

    BasicBlock *latch1 = L1->getLoopLatch();
    BasicBlock *latch2 = L2->getLoopLatch();
    BasicBlock *header1 = L1->getHeader();
    BasicBlock *header2 = L2->getHeader();
    BasicBlock *preheader1 = L1->getLoopPreheader();
    BasicBlock *preheader2 = L2->getLoopPreheader();

    if (!latch1 || !latch2 || !header1 || !header2 || !preheader1 ||
        !preheader2)
    {
        errs() << "Uno dei loop manca di un blocco essenziale.\n";
        return;
    }

    // Modifica del CFG
    // Sostituisci i branch del latch1 e latch2 per la fusione
    Instruction *inst_latch1 = latch1->getTerminator();
    if (!inst_latch1)
    {
        outs() << "Errore: Il latch1 non ha una singola istruzione terminale.\n";
    }
    inst_latch1->replaceUsesOfWith(header1, header2);

    Instruction *inst_latch2 = latch2->getTerminator();
    if (!inst_latch2)
    {
        outs() << "Errore: Il latch2 non ha una singola istruzione terminale.\n";
    }
    inst_latch2->replaceUsesOfWith(header2, header1);

    // Sposta il body del loop2 dopo il body del loop1
    SmallVector<BasicBlock *, 8> body2Blocks(L2->getBlocks());
    for (auto BB = body2Blocks.rbegin(); BB != body2Blocks.rend(); ++BB)
    {
        (*BB)->moveAfter(latch1);
    }

    // TODO: this might be wrong, could invert order of blocks
    //   for (BasicBlock *BB : body2Blocks) {
    //     BB->moveAfter(latch1);
    //   }

    outs()
        << "\n---------- Blocchi del loop2 spostati dopo il latch1 ----------\n";

    // Aggiorna LoopInfo
    for (BasicBlock *BB : body2Blocks)
    {
        LI.changeLoopFor(BB, L1);
    }
    LI.erase(L2);

    outs() << "\n---------- Aggiornamento LoopInfo completato ----------\n";

    // Aggiorna DominatorTree
    DT.recalculate(*header1->getParent());

    outs() << "\n---------- Aggiornamento DominatorTree completato ----------\n";

    // Ora è sicuro rimuovere header2 e latch2.
    header2->eraseFromParent();
    latch2->eraseFromParent();

    outs() << "\n---------- Rimozione header2 e latch2 completata ----------\n";
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
            noNegativeDistanceDependencies(L1, L2, DI) &&
            L1->isLoopSimplifyForm() && L2->isLoopSimplifyForm())
        {
            outs() << "Trovati loop adiacenti candidati per la fusione! \n";
            fusionCandidates.push_back(std::make_pair(L1, L2));
            ++i; // Salta il prossimo loop in quanto è già candidato per la fusion
        }
    }

    for (auto &candidate : fusionCandidates)
    {
        Loop *L1 = candidate.first;
        Loop *L2 = candidate.second;

        replaceInductionVariables(L1, L2);
        fuseLoops(L1, L2, LI, DT);
        outs() << "Fusione dei loop completata.\n";
    }
    outs() << "End loop fusion opt...\n";
    return PreservedAnalyses::all();
}
