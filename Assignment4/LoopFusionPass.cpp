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

/**
 * @brief Verifica di adiacenza di due Loop
 * Si distingue il caso in cui il loop sia guarded da quello non guarded.
 * @param L1 primo loop
 * @param L2 secondo loop
 * @return true
 * @return false
 */
bool areLoopsAdjacent(Loop *l1, Loop *l2) {
  // uso un vettore per mettere tutti i blocchi di uscita del primo loop
  SmallVector<BasicBlock *, 4> exitblock;


  l1->getUniqueNonLatchExitBlocks(exitblock);

  for (BasicBlock *BB : exitblock) {

    // controllo prima se il secondo loop è guarded
    if (l2->isGuarded() &&
        BB != dyn_cast<BasicBlock>(l2->getLoopGuardBranch())) {
      outs() << "Secondo loop Guarded , ma il blocco di uscita del primo loop "
                "non è il blocco di guardia del secondo loop";
      return false;
    }

    if (BB != l2->getLoopPreheader()) {
      outs() << "Secondo loop non Guarded , ma il blocco di uscita del primo "
                "loop non è il preheader del secondo loop";
      return false;
    }
  }
  outs() << "\n> Adiacenti! \n";
  return true;
}

/**
 * @brief Verifica se due loop hanno lo stesso numero di iterazioni.
 *
 * @param L1 Primo loop
 * @param L2 Secondo loop
 * @param SE Oggetto ScalarEvolution per ottenere il numero di iterazioni dei
 * loop
 * @return true Se i due loop hanno lo stesso numero di iterazioni
 * @return false Se i due loop hanno un numero diverso di iterazioni
 */

bool haveSameTripCount(Loop *L1, Loop *L2, ScalarEvolution &SE) {
  const SCEV *S1 = SE.getBackedgeTakenCount(L1);
  const SCEV *S2 = SE.getBackedgeTakenCount(L2);

  if (S1 == S2) {
    outs() << "> I 2 loop hanno lo stesso numero di iterazioni!\n";
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Controlla equivalenza di flusso di controllo tra due loop
 *
 * @param L1 primo loop
 * @param L2 secondo loop
 * @param DT dominator tree della funzione
 * @param PDT post dominator tree della funzione
 * @return true nel caso in cui siano cf equivalenti
 * @return false se non sono cf equivalenti
 */

bool areControlFlowEquivalent(Loop *L1, Loop *L2, DominatorTree &DT,
                              PostDominatorTree &PDT) {

  SmallVector<BasicBlock *> L1exitBlocks;
  L1->getExitBlocks(L1exitBlocks);

  for (BasicBlock *exitBlock : L1exitBlocks) {
    BasicBlock *nextBB = exitBlock->getTerminator()->getSuccessor(0);
    int n_succ = exitBlock->getTerminator()->getNumSuccessors();
    if (n_succ > 0 && !DT.dominates(exitBlock, nextBB) &&
        !PDT.dominates(nextBB, exitBlock)) {
      return false;
    }
  }
  outs() << "> Control Flow equivalent! \n";
  return true;
}

/**
 * @brief Verifica se ci sono dipendenze di distanza Negativa tra due loop, non
 * controlla altri tipi di dipendenze.
 *
 * @param L1 primo loop
 * @param L2 secondo loop
 * @param DI oggetto DependenceInfo per ottenere le informazioni di dipendenza
 * @return true se ci sono dipendenze di distanza negativa
 * @return false se non ci sono dipendenze di distanza negativa
 */

bool hasNegativeDependencies(Loop *L1, Loop *L2, DependenceInfo &DI) {
  // Itera attraverso tutti i basic blocks del loop L1
  for (auto *BB1 : L1->getBlocks()) {
    // Itera attraverso tutti i basic blocks del loop L2
    for (auto *BB2 : L2->getBlocks()) {
      // Itera attraverso tutte le istruzioni di BB1
      for (auto &I1 : *BB1) {
        // Itera attraverso tutte le istruzioni di BB2
        for (auto &I2 : *BB2) {
          // Ottieni l'informazione di dipendenza tra le istruzioni I1 e I2
          auto D = DI.depends(&I1, &I2, true);
          // Se c'è una dipendenza
          if (D) {
            outs() << "> Dipendenza trovata tra " << I1 << " e " << I2 << "\n";
            // Se la dipendenza è di distanza negativa
            // isAnti() indica che la seconda istruzione scrive su una locazione
            // che è stata letta dalla prima istruzione distanza negativa
            if (D->isAnti()) {
              errs() << "> Dipendenza di distanza negativa trovata! \n";
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

/**
 * @brief Sostituisce le variabili di induzione del loop2 con quelle del loop1,
 * e controlla la correttezza della sostituzione.
 *
 * @param L1 primo loop
 * @param L2 secondo loop
 */
void replaceInductionVariables(Loop *L1, Loop *L2) {

  // Ottieni le variabili di induzione per entrambi i loop
  PHINode *indVar1 = L1->getCanonicalInductionVariable();
  PHINode *indVar2 = L2->getCanonicalInductionVariable();

  // Controlla se entrambe le variabili di induzione esistono
  if (!indVar1 || !indVar2) {
    outs() << "Warning: Uno dei loop non ha variabile di induzione.\n";
    return;
  }

  // Sostituisci gli usi della variabile di induzione del loop2 con quelli del
  // loop1
  std::vector<Instruction *> users;
  for (auto &Use : indVar2->uses()) {
    Instruction *User = cast<Instruction>(Use.getUser());
    if (L2->contains(User)) {
      users.push_back(User);
    }
  }

  for (auto *User : users) {
    User->replaceUsesOfWith(indVar2, indVar1);
  }

  // Verifica se la sostituzione è avvenuta correttamente
  bool replacementSuccess = true;
  for (auto &Use : indVar2->uses()) {
    Instruction *User = cast<Instruction>(Use.getUser());
    if (L2->contains(User) && User->getOperand(0) == indVar2) {
      replacementSuccess = false;
      break;
    }
  }

  if (replacementSuccess) {
    outs() << "\n-- Variabili di induzione sostituite con successo "
              "--\n\n";
  } else {
    outs() << "\n\n\n\n\nWARNING: Errore nella sostituzione delle variabili di "
              "induzione.\n\n\n\n\n";
  }
}

/**
 * @brief Fonde due loop che rispettano le 4 condizioni necessarie per la
 * fusione
 *
 * @param L1 primo loop
 * @param L2 secondo loop
 * @return Loop* loop fuso
 */

Loop *fuseLoop(Loop *L1, Loop *L2) {
  outs() << "-- Invocato fuse loop definitivo --\n";

  // Sostituisce le variabili di induzione del loop2 con quelle del loop1
  replaceInductionVariables(L1, L2);

  // prende i blocchi base dei loop di interesse per la fusione
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
  if (L2->contains(header2->getTerminator()->getSuccessor(0))) {
    // successore 0 del branch del header è il body
    body2entry = header2->getTerminator()->getSuccessor(0);
  } else {
    // successore 1 del branch del header è il body
    body2entry = header2->getTerminator()->getSuccessor(1);
  }

  // controlla se preheader2 è uguale all'exit block 1
  assert(preheader2 == exit1 && "preheader2 is not equal to exit2");

  // cambia il successor del header1 (caso terminazione loop) a exit block di L2
  header1->getTerminator()->replaceSuccessorWith(preheader2, exit2);
  // cambia il successor del body1 a body2
  body1->getTerminator()->replaceSuccessorWith(latch1, body2entry);
  // cambia successor del header2 a latch2
  ReplaceInstWithInst(header2->getTerminator(), BranchInst::Create(latch2));

  // cambia il successor del body2 a latch1 per chiudere il loop
  body2->getTerminator()->replaceSuccessorWith(latch2, latch1);

  // Il loop fuso si troverà in L1
  return L1;
}

PreservedAnalyses LoopFusionPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
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

  // Itera attraverso tutti i loop in ordine inverso
  for (auto lit = LI.rbegin(); lit != LI.rend(); ++lit) {
    Loop *L2 = *lit;

    if (L1) {
      if (areLoopsAdjacent(L1, L2) && haveSameTripCount(L1, L2, SE) &&
          areControlFlowEquivalent(L1, L2, DT, PDT) &&
          !hasNegativeDependencies(L1, L2, DI) && L1->isLoopSimplifyForm() &&
          L2->isLoopSimplifyForm()) {
        outs() << "Trovati loop adiacenti candidati per la fusione! \n";
        fuseLoop(L1, L2);
        outs() << "\n-- Fusione dei loop completata con successo"
                  "--\n\n";
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