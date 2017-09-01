//
//
//

#include "Config.hpp"

#include "Utils.hpp"

#include "DecoupleLoopsFrontPass.hpp"

#if DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS
#include "DecoupleLoops.h"
#endif // DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS

#include "llvm/Pass.h"
// using llvm::RegisterPass

#include "llvm/IR/Type.h"
// using llvm::Type

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Dominators.h"
// using llvm::DominatorTreeWrapperPass
// using llvm::DominatorTree

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass
// using llvm::LoopInfo

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::list
// using llvm::cl::desc
// using llvm::cl::value_desc
// using llvm::cl::location
// using llvm::cl::ZeroOrMore

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/FileSystem.h"
// using llvm::sys::fs

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include <set>
// using std::set

#include <algorithm>
// using std::for_each

#include <string>
// using std::string
// using std::stoul

#include <fstream>
// using std::ifstream

#include <cassert>
// using assert

#define DEBUG_TYPE "decouple-loops-front"

#define STRINGIFY_UTIL(x) #x
#define STRINGIFY(x) STRINGIFY_UTIL(x)

#define PRJ_CMDLINE_DESC(x) x " (version: " STRINGIFY(VERSION_STRING) ")"

// plugin registration for opt

namespace icsa {

char DecoupleLoopsFrontPass::ID = 0;
static llvm::RegisterPass<DecoupleLoopsFrontPass>
    X("decouple-loops-front", PRJ_CMDLINE_DESC("decouple loops front pass"),
      false, false);

// plugin registration for clang

// the solution was at the bottom of the header file
// 'llvm/Transforms/IPO/PassManagerBuilder.h'
// create a static free-floating callback that uses the legacy pass manager to
// add an instance of this pass and a static instance of the
// RegisterStandardPasses class

static void
registerDecoupleLoopsFrontPass(const llvm::PassManagerBuilder &Builder,
                               llvm::legacy::PassManagerBase &PM) {
  PM.add(new DecoupleLoopsFrontPass());

  return;
}

static llvm::RegisterStandardPasses
    RegisterDecoupleLoopsFrontPass(llvm::PassManagerBuilder::EP_EarlyAsPossible,
                                   registerDecoupleLoopsFrontPass);

//

static llvm::cl::opt<std::string>
    ReportFilenamePrefix("dlf-report",
                         llvm::cl::desc("report filename prefix"));

#if DECOUPLELOOPSFRONT_DEBUG
bool passDebugFlag = false;
static llvm::cl::opt<bool, true>
    Debug("dlf-debug", llvm::cl::desc("debug decouple loops front pass"),
          llvm::cl::location(passDebugFlag));
#endif // DECOUPLELOOPSFRONT_DEBUG

namespace {

void checkCmdLineOptions(void) { return; }

void report(llvm::StringRef FilenamePrefix, llvm::StringRef FilenameSuffix) {
  std::error_code err;

  auto filename = FilenamePrefix.str() + FilenameSuffix.str() + ".txt";
  llvm::raw_fd_ostream report(filename, err, llvm::sys::fs::F_Text);

  if (err)
    llvm::errs() << "could not open file: \"" << filename
                 << "\" reason: " << err.message() << "\n";
  else {
  }

  report.close();

  return;
}

} // namespace anonymous end

//

bool DecoupleLoopsFrontPass::runOnModule(llvm::Module &CurMod) {
  checkCmdLineOptions();

  bool hasModuleChanged = false;
  bool shouldReport = !ReportFilenamePrefix.empty();
  llvm::SmallVector<llvm::Loop *, 16> workList;

  for (auto &CurFunc : CurMod) {
    if (CurFunc.isDeclaration())
      continue;

    auto &DT =
        getAnalysis<llvm::DominatorTreeWrapperPass>(CurFunc).getDomTree();
    auto &DLP = getAnalysis<DecoupleLoopsPass>(CurFunc);
    auto &LI = *DLP.getLI(&CurFunc);

    workList.clear();

    auto loopsFilter = [&](auto *e) { workList.push_back(e); };

    std::for_each(LI.begin(), LI.end(), loopsFilter);

    std::reverse(workList.begin(), workList.end());

    for (auto *e : workList) {
      llvm::SmallVector<llvm::BasicBlock *, 16> bbWorkList;
      bbWorkList.append(e->block_begin(), e->block_end());
    }
  }

  if (shouldReport) {
  }

  return hasModuleChanged;
}

void DecoupleLoopsFrontPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequiredTransitive<llvm::LoopInfoWrapperPass>();
  AU.addPreserved<llvm::LoopInfoWrapperPass>();
  AU.addRequired<llvm::DominatorTreeWrapperPass>();
  AU.addRequired<DecoupleLoopsPass>();

  return;
}
} // namespace icsa end
