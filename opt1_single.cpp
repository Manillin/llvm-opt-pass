//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
// L'include seguente va in LocalOpts.h
#include <llvm/IR/Constants.h>

using namespace llvm;

bool runOnBasicBlock(BasicBlock &B) {

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


bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}


PreservedAnalyses LocalOpts::run(Module &M,
                                      ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();
  
  return PreservedAnalyses::all();
}
