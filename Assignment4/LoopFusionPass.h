#ifndef LLVM_TRANSFORMS_LOOPFUSIONPASS_H
#define LLVM_TRANSFORMS_LOOPFUSIONPASS_H

#include "llvm/IR/PassManager.h"

namespace llvm
{
    class LoopFusionPass : public PassInfoMixin<LoopFusionPass>
    {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    };
} // namespace llvm

#endif // LLVM_TRANSFORMS_LOOPFUSIONPASS_H