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
// L'include seguente va in LocalOpts.h
#include "llvm/Transforms/Utils/Local.h"
#include <llvm/IR/Constants.h>

using namespace llvm;

bool strengthReduction(Instruction &I){  /*CHRISTIAN INSERT CODE HERE */ return true}


bool algebricIdentity(BasicBlock &B){

    std::vector<Instruction*> toDelete;
    Value *op1,*op2;
    ConstantInt *cost1, *cost2;
    //prendo tutte le istruzioni e controllo che queste istruzioni siano somme oppure moltiplicazioni
    for(auto &I : B){ 
        //può essere letta come "deduci il tipo di BinOp basandoti sul risultato del casting dinamico di &Inst a BinaryOperator"
        if(auto *Binop = dyn_cast<BinaryOperator>(&I)){ //se è un istruzione binaria
            //estraggo operandi
            op1=Binop->getOperand(0);
            op2=Binop->getOperand(1);
            //controllo se gli operandi sono costanti intere , avrò null se non saranno costanti intere
            cost1=dyn_cast<ConstantInt>(op1);
            cost2=dyn_cast<ConstantInt>(op2);

/*bisogna controllare anche che una delle due variabili non sia NULL nel caso in cui non sia null allora dobbiamo controllare
se la costante è uguale a 0 nel caso dell'add invece che la costante sia uguale a 1 nel caso della Mul*/

//////////////////////ADD/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ricorda che l'add si puo fare : add rd,rs1,rs2 

            if(Binop->getOpcode() == Instruction::Add){ //controllo che sia una somma
                outs() << "Trovata istruzione binaria add: " << *Binop << "\n";

            //controllo che il primo operando sia 0
                if(cost1 != NULL && cost1->isZero()){
                    outs() << "Trovata istruzione addizione con primo operando 0: " << *Binop << "\n";
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op2); //sostituisco tutte le occorrenze di un valore con un altro valore
                }
                else if (cost2 != NULL && cost2->isZero()){
                    outs() << "Trovata istruzione addizione con secondo operando 0: " << *Binop << "\n";
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op1);
                }
            }
            

            

////////////////////////MUL/////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if(Binop->getOpcode() == Instruction::Mul){//controllo che sia una moltiplicazione
            //controllo che il primo o secondo operando sia 1
                if(cost1 != NULL && cost1->isOne()){
                    outs() << "Trovata istruzione moltiplicazione con primo operando 1: " << *Binop << "\n";
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op2);
                }
                else if (cost2 != NULL && cost2->isOne()){
                    outs() << "Trovata istruzione moltiplicazione con secondo operando 1: " << *Binop << "\n";
                    toDelete.push_back(Binop);
                    Binop->replaceAllUsesWith(op1);
                }
            }
        }
    }    

    //Cancellazione di tutte le istruzioni inutili
    for (auto& element : toDelete) {
        outs()<<"Cancello la seguente istruzione : "<<*element<<"\n";
        element->eraseFromParent();
    }


    return true;
}

Value *findOperator(BasicBlock::iterator sottrazione,
                    BasicBlock::iterator primaIstruzione, Value *var,
                    const llvm::APInt costanteSub)
{

    // Itera le istruzioni partendo dalla sottrazione e arriva fino all'inizio (primaIstruzione)              
    BasicBlock::iterator it = sottrazione;
    ConstantInt *C0, *C1;
    do
    {
        if (it == primaIstruzione)
            break;
        it--;

        Instruction *sub = dyn_cast<Instruction>(var);
        Instruction *instruction = &(*it);

        // Controllo se il value dell'operazione che sto analizzando è uguale al value del sottrazione     
        if (sub->getOperand(0) == instruction->getOperand(0) &&
            sub->getOperand(1) == instruction->getOperand(1))
        {
            outs() << "Ho trovato un istruzione con il Value che è un buon candidato "
                      "( istr : "
                   << *instruction << " )\n";
            C0 = dyn_cast<ConstantInt>(instruction->getOperand(0));
            C1 = dyn_cast<ConstantInt>(instruction->getOperand(1));
            if (C0 != NULL)
            {
                const llvm::APInt costanteAdd = C0->getValue();
                outs() << "Costante trovata! : " << costanteAdd << "\n";
                if (costanteAdd.eq(costanteSub))
                {
                    outs() << "Costante uguale a quella della sottrazione!\n";
                    return instruction->getOperand(1);
                }
            }
            else if (C1 != NULL)
            {
                const llvm::APInt costanteAdd = C1->getValue();
                outs() << "Costante trovata! : " << costanteAdd << "\n";
                if (costanteAdd.eq(costanteSub))
                {
                    outs() << "Costante uguale a quella della sottrazione! : \n";
                    return instruction->getOperand(0);
                }
            }
            else
            {
                outs() << "Nessuna costante trovata!\n";
            }
        }
    } while (true);

    return NULL;
}

void multi_instr_opt(BasicBlock &B)
{
    unsigned cont = 0;
    std::vector<Instruction *> toDelete;

    // Itera tutte le istruzioni
    for (auto iter_i = B.begin(); iter_i != B.end(); ++iter_i)
    {
        cont++;
        Instruction &I = *iter_i;

        // Controllo se l'istruzione è una sottrazione
        if (I.isBinaryOp() && I.getOpcode() == Instruction::Sub)
        {

            BinaryOperator *sub = dyn_cast<BinaryOperator>(&I);
            outs() << "(" << *sub << ")"
                   << " è una sottrazione\n";

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
            if (C)
            {
                const llvm::APInt costanteIntera = C->getValue();
                Value *new_value =
                    findOperator(iter_i, B.begin(), variabile, costanteIntera);
                if (new_value)
                { // Controllo se ho trovato un addizione con le
                  // caratteristiche desiderate
                    outs() << "Nuovo valore che devo mettere  c = ... <----- = {"
                           << *new_value << "} \n";
                    I.replaceAllUsesWith(new_value);
                    toDelete.push_back(&I);
                }
                else
                {
                    outs() << "Non ho trovato nessuna <Add> con le caratteristiche "
                              "adatte!\n";
                }
            }
            else
            {
                outs() << "La seguente sottrazione NON ha una costante intera\n\n";
            }
            outs() << "\n\n";
        }
    }

    // Cancellazione di tutte le istruzioni inutili
    for (auto &element : toDelete)
    {
        outs() << "Cancello la seguente istruzione : " << *element << "\n";
        element->eraseFromParent();
    }

    // Debug
    outs() << "\nIstruzioni analizzate : " << cont << "\n";
}

bool runOnBasicBlock(BasicBlock &B)
{
    algebricIdentity(B); // chiama l'ottimizzatore del punto 1
    multi_instr_opt(B);  // chiama l'ottimizatore del punto3

    return true;
}

bool runOnFunction(Function &F)
{
    bool Transformed = false;

    for (auto Iter = F.begin(); Iter != F.end(); ++Iter)
    {
        if (runOnBasicBlock(*Iter))
        {
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