//
//
//

#ifndef DECOUPLELOOPSFRONTPASS_HPP
#define DECOUPLELOOPSFRONTPASS_HPP

#include "Config.hpp"

#include "llvm/Pass.h"
// using llvm::ModulePass

namespace llvm {
class Module;
} // namespace llvm end

namespace icsa {

class DecoupleLoopsFrontPass : public llvm::ModulePass {
public:
  static char ID;

  DecoupleLoopsFrontPass() : llvm::ModulePass(ID) {}
  bool runOnModule(llvm::Module &CurMod) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

} // namespace icsa end

#endif // header
