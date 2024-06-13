//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"


using namespace llvm;

 
/// @brief Function that find algebric identity. If exists it replace all useless uses.
/// @param B BasicBlock where you search the algebric identity
/// @return  True -> if the function made the optimization
bool algebricIdentity(BasicBlock &B)
{

    std::vector<Instruction *> toDelete;
    Value *op1, *op2;
    ConstantInt *cost1, *cost2;
    // prendo tutte le istruzioni e controllo che queste istruzioni siano somme oppure moltiplicazioni
    for (auto &I : B)
    {
        // può essere letta come "deduci il tipo di BinOp basandoti sul risultato del casting dinamico di &Inst a BinaryOperator"
        if (auto *Binop = dyn_cast<BinaryOperator>(&I))
        { // se è un istruzione binaria
            // estraggo operandi
            op1 = Binop->getOperand(0);
            op2 = Binop->getOperand(1);
            // controllo se gli operandi sono costanti intere , avrò null se non saranno costanti intere
            cost1 = dyn_cast<ConstantInt>(op1);
            cost2 = dyn_cast<ConstantInt>(op2);

            /*bisogna controllare anche che una delle due variabili non sia NULL nel caso in cui non sia null allora dobbiamo controllare
            se la costante è uguale a 0 nel caso dell'add invece che la costante sia uguale a 1 nel caso della Mul*/

            //////////////////////ADD/////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // ricorda che l'add si puo fare : add rd,rs1,rs2

            if (Binop->getOpcode() == Instruction::Add) { // controllo che sia una somma
                //outs() << "Trovata istruzione binaria add: " << *Binop << "\n";

                // controllo che il primo operando sia 0
                if (cost1 != NULL && cost1->isZero())
                {
                    //outs() << "Trovata istruzione addizione con primo operando 0: " << *Binop << "\n";
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op2); // sostituisco tutte le occorrenze di un valore con un altro valore
                }
                else if (cost2 != NULL && cost2->isZero()){
                    //outs() << "Trovata istruzione addizione con secondo operando 0: " << *Binop << "\n";
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op1);
                }
            }

            ////////////////////////MUL/////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if (Binop->getOpcode() == Instruction::Mul)
            { // controllo che sia una moltiplicazione
                // controllo che il primo o secondo operando sia 1
                if (cost1 != NULL && cost1->isOne()){
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op2);
                }
                else if (cost2 != NULL && cost2->isOne()){
                    
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op1);
                }
            }
        }
    }

    // Cancellazione di tutte le istruzioni inutili
    for (auto &element : toDelete){
        element->eraseFromParent();
    }

    return true;
}

/**
 * @brief Get the Best Shift Value for a constant
 *
 * @param constVal constant
 * @return unsigned int
 */

unsigned int getBestShiftValue(uint64_t constVal){

    // verifica se potenza di 2:
    APInt apInt(32, constVal);
    if (apInt.isPowerOf2()){
        return apInt.logBase2();
    }
    // verifica se potenza di due con offset +1
    APInt apIntPlusOne(32, constVal + 1);
    if (apIntPlusOne.isPowerOf2()){
        return apIntPlusOne.logBase2();
    }
    // verifica se potenza di due con differenza di -1
    APInt apIntMinusOne(32, constVal - 1);
    if (apIntMinusOne.isPowerOf2()){
        return apIntMinusOne.logBase2();
    }

    return 0;
}



/// @brief   Function that try to do "strength reduction" opt for an instruction. 
/// @param I Instruction where you search the algebric identity
/// @return  True -> if the function made the optimization
bool strengthReduction(Instruction &I) {
    if (auto *BinOp = dyn_cast<BinaryOperator>(&I))
    {
        auto OpCode = BinOp->getOpcode();
        Value *Op1 = I.getOperand(0);
        Value *Op2 = I.getOperand(1);

        if (ConstantInt *constInt = dyn_cast<ConstantInt>(Op1))
            std::swap(Op1, Op2);

        if (!isa<ConstantInt>(Op2))
        {
            // Op2 non contiene una costante -> errore
            return false;
        }

        // DBUG: outs() << "Assegnamento variabili corretto!\n";
        ConstantInt *constInt = dyn_cast<ConstantInt>(Op2);
        // calcolo shift value

        unsigned int shiftValue = getBestShiftValue(constInt->getZExtValue());
        ConstantInt *shift = ConstantInt::get(constInt->getType(), shiftValue);

        // DBUG: outs() << "shiftValue: " << shiftValue << "\n";

        Instruction *newInstruction = nullptr;
        if (OpCode == BinaryOperator::Mul)
        {
            if (!Op1)
            {
                //outs() << "OP1 riferimento nullo\n";
                return false;
            }
            Instruction *shiftLeft =
                BinaryOperator::Create(Instruction::Shl, Op1, shift);

            // Verifica che shiftLeft sia stato creato correttamente
            if (!shiftLeft)
            {
                //outs() << "Errore: impossibile creare shiftLeft\n";
                return false;
            }
            // DBUG: outs() << "shiftLeft:  -> " << *shiftLeft << "\n";
            shiftLeft->insertAfter(&I);

            // calcolo del resto
            int64_t operationRest =
                static_cast<int64_t>(constInt->getZExtValue()) - (1 << shiftValue);
            //outs() << "Triggered Strenght Reduction on " << I << "\n";

            // analisi del resto
            if (operationRest == 0)
            {
                newInstruction = shiftLeft;
            }
            else if (operationRest == 1)
            {
                newInstruction =
                    BinaryOperator::Create(BinaryOperator::Add, shiftLeft, Op1);
                //outs() << "newInstruction: " << *newInstruction << "\n";
                newInstruction->insertAfter(shiftLeft);
            }
            else if (operationRest == -1)
            {
                newInstruction =
                    BinaryOperator::Create(BinaryOperator::Sub, shiftLeft, Op1);
                //outs() << "newInstruction: " << *newInstruction << "\n";
                newInstruction->insertAfter(shiftLeft);
            }
            else
            {
                // Il resto non è 0, 1 o -1, quindi non eseguire la strength reduction
                return false;
            }
        }

        else if (OpCode == BinaryOperator::UDiv)
        {
            if (constInt->getValue().isPowerOf2())
            {
                newInstruction = BinaryOperator::Create(Instruction::LShr, Op1, shift);
                //outs() << "newInstruction: " << *newInstruction << "\n";
                newInstruction->insertAfter(&I);
            }
        }
        if (newInstruction)
            I.replaceAllUsesWith(newInstruction);

        return newInstruction;
    }
    else
    {
        return false;
    }
}

/// @brief Function that finds the new value of the instruction starting from sub_istr (backwards) 
/// @param sottrazione  Sub candidate (ex: c=a-1)
/// @param primaIstruzione  First instruction of basicblock.
/// @param var Variable in the subtraction(ex:a)
/// @param costanteSub Constant in the subtraction(ex:1)
/// @return  new value found
Value *findOperator(BasicBlock::iterator sottrazione, BasicBlock::iterator primaIstruzione, Value *var, const llvm::APInt costanteSub){

    // Itera le istruzioni partendo dalla sottrazione e arriva fino all'inizio (primaIstruzione)
    BasicBlock::iterator it = sottrazione;
    ConstantInt *C0, *C1;
    do{
        if (it == primaIstruzione)
            break;
        it--;

        Instruction *sub = dyn_cast<Instruction>(var);
        Instruction *instruction = &(*it);

        // Controllo se il value dell'operazione che sto analizzando è uguale al value del sottrazione
        if (sub->getOperand(0) == instruction->getOperand(0) && sub->getOperand(1) == instruction->getOperand(1)){
            //outs() << "Ho trovato un istruzione con il Value che è un buon candidato ""( istr : " << *instruction << " )\n";
            C0 = dyn_cast<ConstantInt>(instruction->getOperand(0));
            C1 = dyn_cast<ConstantInt>(instruction->getOperand(1));
            if (C0 != NULL){
                const llvm::APInt costanteAdd = C0->getValue();
                if (costanteAdd.eq(costanteSub)){
                    //outs() << "Costante uguale a quella della sottrazione!\n";
                    return instruction->getOperand(1);
                }
            }else if (C1 != NULL){
                const llvm::APInt costanteAdd = C1->getValue();
                if (costanteAdd.eq(costanteSub)){
                    //outs() << "Costante uguale a quella della sottrazione! : \n";
                    return instruction->getOperand(0);
                }
            }
        }
    } while (true);

    return NULL;
}

/// @brief Function that find "multi instructions operations". If exists it replace all useless uses.
/// @param B  BasicBlock where you search the "multi instructions"
/// @return  True -> if the function made the optimization
bool multi_instr_opt(BasicBlock &B){
    unsigned cont = 0;
    std::vector<Instruction *> toDelete;
    bool opt_done = false;

    // Itera tutte le istruzioni
    for (auto iter_i = B.begin(); iter_i != B.end(); ++iter_i){
        cont++;
        Instruction &I = *iter_i;

        // Controllo se l'istruzione è una sottrazione
        if (I.isBinaryOp() && I.getOpcode() == Instruction::Sub){

            BinaryOperator *sub = dyn_cast<BinaryOperator>(&I);
            Value *op_0 = sub->getOperand(0);
            Value *op_1 = sub->getOperand(1);

            ConstantInt *C;
            Value *variabile;
            if ((C = dyn_cast<ConstantInt>(op_1)))
            {
                variabile = op_0;
            }
            else if ((C = dyn_cast<ConstantInt>(op_0)))
            {
                variabile = op_1;
            }

            // Controllo se ho trovato una costante
            if (C){
                const llvm::APInt costanteIntera = C->getValue();
                Value *new_value = findOperator(iter_i, B.begin(), variabile, costanteIntera);
                if (new_value){     // Controllo se ho trovato un addizione con le caratteristiche desiderate
                    //outs() << "Nuovo valore che devo mettere  c = ... <----- = {"<< *new_value << "} \n";
                    I.replaceAllUsesWith(new_value);
                    toDelete.push_back(&I);
                }
            }
        }
    }

    // Cancellazione di tutte le istruzioni inutili
    for (auto &element : toDelete){
        element->eraseFromParent();
        opt_done = true;
    }

    // Debug
    //outs() << "\nIstruzioni analizzate : " << cont << "\n";

    return opt_done;
}



// Main
bool runOnBasicBlock(BasicBlock &B){
    // chiama l'ottimizzatore del punto 1
    if(algebricIdentity(B))
        outs()<<"- Algebric Identity ✓ \n";

    // ottimizzatore punto 2
    unsigned count_strengh = 0;
    std::vector<Instruction *> toRemove;
    for (auto &I : B){
        if (strengthReduction(I)){
            toRemove.push_back(&I);
            count_strengh++;
        }
    }
    // Rimuovi le istruzioni dopo aver completato il ciclo
    for (auto *I : toRemove){
        I->eraseFromParent();
    }
    if(count_strengh > 0)
        outs()<<"- Strengh Reduction ✓ ("<<count_strengh<<" volte) \n";

    // chiama l'ottimizatore del punto3
    if(multi_instr_opt(B))
        outs()<<"- Multi Instructions ✓ \n";

    return true;
}

bool runOnFunction(Function &F)
{
    bool Transformed = false;

    // Try to optimaze each BasicBlock of the function
    for (auto Iter = F.begin(); Iter != F.end(); ++Iter){
        if (runOnBasicBlock(*Iter)){
            Transformed = true;
        }
    }
    return Transformed;
}



PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM)
{
    for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
        if (runOnFunction(*Fiter))
            return PreservedAnalyses::none();

    return PreservedAnalyses::all();
}

