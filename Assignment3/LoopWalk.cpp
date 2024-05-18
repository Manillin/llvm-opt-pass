#include "llvm/Transforms/Utils/LoopWalk.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include <llvm/IR/Constants.h>

using namespace llvm;

// sposta istruzioni nel preheader:

void move_to_preheader(Loop *L,
                       std::set<Instruction *> &loop_invariant_instructions)
{
    BasicBlock *preheader = L->getLoopPreheader();
    Instruction *terminator = preheader->getTerminator();

    for (Instruction *inst : loop_invariant_instructions)
    {
        inst->moveBefore(terminator);
    }

    outs() << "Moved Loop Invariant Instructions to preheader!\n";
}

// Verifica se un istruzione è stata definita esternamente.
bool isDefinedOutside(Value *Operand, Loop &L)
{
    Instruction *I_temp = dyn_cast<Instruction>(Operand);
    if (I_temp != NULL)
    {
        return (L.contains(I_temp->getParent()));
    }
    else
    {
        return false;
    }
}

// Verifica se un operando è una costante.
bool isCostant(Value *Operand)
{
    if (ConstantInt *C = dyn_cast<ConstantInt>(Operand))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Verifica se un operando è valido per rendere l'istruzione Loop Invariant
bool isLoopInvariantCandidate(
    Value *Operand, Loop &L,
    std::set<Instruction *> loop_invariant_instructions)
{
    Instruction *I_link = dyn_cast<Instruction>(Operand);
    return (isCostant(Operand) || isDefinedOutside(Operand, L) ||
            I_link != NULL || (loop_invariant_instructions.count(I_link) > 0) ||
            (isa<Argument>(Operand)));
}

// Trova i blocchi di uscita del ciclo
std::set<BasicBlock *> find_exit_blocks(Loop &L)
{
    std::set<BasicBlock *> exit_blocks;
    for (BasicBlock *BB : L.blocks())
    {
        for (Instruction &I : *BB)
        {
            if (BranchInst *BI = dyn_cast<BranchInst>(&I))
            {
                for (unsigned i = 0; i < BI->getNumSuccessors(); ++i)
                {
                    BasicBlock *succ = BI->getSuccessor(i);
                    if (!L.contains(succ))
                    {
                        exit_blocks.insert(succ);
                    }
                }
            }
        }
    }
    return exit_blocks;
}

// Trova i blocchi che dominano TUTTE le uscite
std::set<BasicBlock *> find_exit_dominators(DominatorTree &DT, Loop &L)
{
    std::set<BasicBlock *> exit_blocks = find_exit_blocks(L);
    std::set<BasicBlock *> exit_dominators;

    for (BasicBlock *BB : L.blocks())
    {
        bool dominates_all_exits = true;

        for (BasicBlock *exitBlock : exit_blocks)
        {
            if (!DT.dominates(BB, exitBlock))
            {
                dominates_all_exits = false;
                break;
            }
        }
        if (dominates_all_exits)
        {
            exit_dominators.insert(BB);
        }
    }
    return exit_dominators;
}

// Controlla se un'istruzione domina TUTTI i suoi usi
// TODO: NON può essere passata su un'istruzione che non sia binaria (?), aggiungi controlli
bool instruction_dominates_all_uses(Instruction *I, DominatorTree &DT,
                                    Loop &L)
{
    for (User *U : I->users())
    {
        Instruction *userInst = dyn_cast<Instruction>(U);
        if (userInst && L.contains(userInst->getParent()))
        {
            if (!DT.dominates(I, userInst))
            {
                return false;
            }
        }
    }
    return true;
}

// TODO: check se non istruzione binaria
bool is_dead(Instruction *I, Loop &L)
{
    for (User *U : I->users())
    {
        Instruction *userInst = dyn_cast<Instruction>(U);
        if (userInst && !L.contains(userInst->getParent()))
        {
            return false;
        }
    }
    return true;
}

PreservedAnalyses LoopWalk::run(Loop &L, LoopAnalysisManager &LAM,
                                LoopStandardAnalysisResults &LAR,
                                LPMUpdater &LU)
{

    outs() << "Starting loop programm: \n\n";

    std::set<Instruction *> loop_invariant_instructions;

    if (!L.isLoopSimplifyForm())
    {
        outs() << "\n il Loop non è in forma normale \n";
        return PreservedAnalyses::all();
    }
    BasicBlock *preheader = L.getLoopPreheader();
    if (!preheader)
    {
        errs() << "Il loop NON ha un Preheader!\n";
        return PreservedAnalyses::all();
    }
    outs() << "\nIl loop è in forma Normale e ha un Preheader!\nContinuo...\n";

    BasicBlock *head = L.getHeader();
    Function *F = head->getParent(); // recuperiamo l'handle alla funzione che
                                     // contiene il Loop

    // TODO: modificare in forma for(auto *BB: ...)
    for (auto BI = L.block_begin(); BI != L.block_end(); ++BI)
    {

        BasicBlock &BB = **BI;

        for (auto &I : BB)
        {
            // outs() << "Analisi dell'istruzione : " << I << " : \n";
            bool isLoopInvariant = true;
            PHINode *phi_node = dyn_cast<PHINode>(&I);

            // Check if is phi_instruction
            if (phi_node)
            {
                isLoopInvariant = false;
            }
            else
            {
                for (auto *Iter = I.op_begin(); Iter != I.op_end(); ++Iter)
                {
                    Value *Operand = *Iter;
                    // outs() << "op : " << *Operand << "\n"; -> debug print

                    if (!isLoopInvariantCandidate(Operand, L,
                                                  loop_invariant_instructions))
                        isLoopInvariant = false;
                }
            }

            if (isLoopInvariant == true)
            {
                loop_invariant_instructions.insert(&I);
            }
        }
    }

    outs() << "\nLoop invariant instructions : \n";
    for (const auto &element : loop_invariant_instructions)
    {
        outs() << *element << "\n";
    }

    outs() << "\nAttempting to move instructions...\n";
    move_to_preheader(&L, loop_invariant_instructions);

    return PreservedAnalyses::all();
}
