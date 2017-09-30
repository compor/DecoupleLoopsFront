//
//
//

#include "Config.hpp"

#include "Utils.hpp"

#include "DecoupleLoopsFront.hpp"

#include "DecoupleLoopsFrontPass.hpp"

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
#include "AnnotateLoops.hpp"
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

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

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
// using llvm::SplitBlock

#include "llvm/Transforms/Scalar.h"
// using char llvm::LoopInfoSimplifyID

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/GraphWriter.h"
// using llvm:WriteGraph

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

#include <vector>
// using std::vector

#include <map>
// using std::map

#include <set>
// using std::set

#include <algorithm>
// using std::for_each
// using std::reverse

#include <string>
// using std::string
// using std::stoul

#include <fstream>
// using std::ifstream

#include <system_error>
// using std::error_code

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

static llvm::cl::opt<bool>
    DotCFGOnly("dlf-dot-cfg-only",
               llvm::cl::desc("print CFG of altered functions to 'dot' file "
                              "(with no function bodies)"));

static llvm::cl::opt<std::string>
    DotDirectory("dlf-dot-dir",
                 llvm::cl::desc("location to output 'dot' files"),
                 llvm::cl::init("."));

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

using FunctionName_t = std::string;

std::set<FunctionName_t> FunctionsAltered;

void report(llvm::StringRef FilenamePrefix, llvm::StringRef FilenameSuffix) {
  std::error_code err;

  auto filename = FilenamePrefix.str() + FilenameSuffix.str() + ".txt";
  llvm::raw_fd_ostream report(filename, err, llvm::sys::fs::F_Text);

  if (err)
    llvm::errs() << "could not open file: \"" << filename
                 << "\" reason: " << err.message() << "\n";
  else {
    for (const auto &e : FunctionsAltered)
      report << e << "\n";
  }

  report.close();

  return;
}

} // namespace anonymous end

//

bool DecoupleLoopsFrontPass::runOnModule(llvm::Module &CurMod) {
  using ModeChangePointTy =
      std::pair<llvm::Instruction *, IteratorRecognitionMode>;
  std::map<llvm::BasicBlock *, std::vector<ModeChangePointTy>> modeChanges;
  std::map<llvm::BasicBlock *, IteratorRecognitionMode> bbModes;
  bool hasModuleChanged = false;
  bool hasFunctionChanged = false;
  bool shouldReport = !ReportFilenamePrefix.empty();
  llvm::SmallVector<llvm::Loop *, 16> workList;

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
  std::set<unsigned int> loopIDs;
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

  for (auto &CurFunc : CurMod) {
    hasFunctionChanged = false;
    workList.clear();
    modeChanges.clear();
    bbModes.clear();

    if (CurFunc.isDeclaration())
      continue;

    DEBUG_CMD(llvm::errs() << "process func: " << CurFunc.getName() << "\n");

    auto &DT =
        getAnalysis<llvm::DominatorTreeWrapperPass>(CurFunc).getDomTree();
    auto &LI = getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc).getLoopInfo();
    auto &DLP = getAnalysis<DecoupleLoopsPass>(CurFunc);
    auto &DLPLI = *DLP.getLI(&CurFunc);

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
    AnnotateLoops al;

    auto loopsFilter = [&](auto *e) {
      if (al.hasAnnotatedId(*e)) {
        auto id = al.getAnnotatedId(*e);
        workList.push_back(e);
      }
    };
#else
    auto loopsFilter = [&](auto *e) { workList.push_back(e); };
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

    std::for_each(DLPLI.begin(), DLPLI.end(), loopsFilter);
    std::reverse(workList.begin(), workList.end());

    unsigned numLoopsToAlterPerFunction = 0;
    unsigned lastIdNum = 0;

    for (auto *e : workList) {
      if (!DLP.hasWork(e))
        continue;

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
      if (al.hasAnnotatedId(*e))
        lastIdNum = al.getAnnotatedId(*e);
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

      llvm::SmallVector<llvm::BasicBlock *, 16> bbWorkList;
      bbWorkList.append(e->block_begin(), e->block_end());

      for (auto *bb : bbWorkList) {
        auto *firstI = bb->getFirstNonPHI();

        IteratorRecognitionMode lastMode = GetMode(*firstI, *e, DLP);
        bool hasAllSameModeInstructions = true;

        for (llvm::BasicBlock::iterator ii = bb->getFirstNonPHI(),
                                        ie = bb->end();
             ii != ie; ++ii) {
          auto &inst = *ii;
          IteratorRecognitionMode instMode = GetMode(inst, *e, DLP);

          if (lastMode == instMode)
            continue;

          hasAllSameModeInstructions = false;
          auto modeChangePt = std::make_pair(&inst, instMode);
          if (modeChanges.find(bb) == modeChanges.end())
            modeChanges.emplace(bb, std::vector<ModeChangePointTy>{});

          modeChanges.at(bb).push_back(modeChangePt);
          lastMode = instMode;
        }

        if (hasAllSameModeInstructions)
          bbModes.emplace(bb, lastMode);
        else
          numLoopsToAlterPerFunction++;
      }
    }

    // xform part
    // clang-format off
    DEBUG_CMD(if (modeChanges.size())
              llvm::errs() << "transform func: " << CurFunc.getName() << "\n");
    // clang-format on

    for (auto &e : modeChanges) {
      auto *oldBB = e.first;
      IteratorRecognitionMode mode;
      std::reverse(e.second.begin(), e.second.end());
      for (auto &k : e.second) {
        auto *splitI = k.first;
        mode = k.second;
        auto *newBB = llvm::SplitBlock(oldBB, splitI, &DT, &LI);
        hasModuleChanged |= true;
        hasFunctionChanged = true;
        bbModes.emplace(newBB, mode);

        if (shouldReport)
          FunctionsAltered.insert(CurFunc.getName());
      }

      bbModes.emplace(oldBB, InvertMode(mode));
    }

    for (auto &e : bbModes) {
      llvm::StringRef name = "";
      if (e.first->hasName()) {
        name = e.first->getName();
      }

      llvm::StringRef prefix;
      e.second == IteratorRecognitionMode::PayloadMode ? prefix = "pd_"
                                                       : prefix = "it_";

      e.first->setName(prefix + name);
    }

    if (DotCFGOnly && hasFunctionChanged) {
      std::string extraId{""};

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
      if (numLoopsToAlterPerFunction == 1)
        extraId = "." + std::to_string(lastIdNum);
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

      auto dotFilename =
          (DotDirectory + "/cfg." + CurFunc.getName() + extraId + ".dot").str();

      DEBUG_CMD(llvm::errs() << "writing file: " << dotFilename << "\n");
      std::error_code ec;
      llvm::raw_fd_ostream dotFile(dotFilename, ec, llvm::sys::fs::F_Text);

      if (!ec)
        llvm::WriteGraph(dotFile, (const llvm::Function *)&CurFunc,
                         llvm::sys::fs::F_Text);
      else
        DEBUG_CMD(llvm::errs() << "error writing file: " << dotFilename
                               << "\n");
    }
  }

  if (shouldReport)
    report(ReportFilenamePrefix, "FunctionsAltered");

  return hasModuleChanged;
}

void DecoupleLoopsFrontPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addPreservedID(llvm::LoopSimplifyID);
  AU.addRequiredTransitive<llvm::LoopInfoWrapperPass>();
  AU.addPreserved<llvm::LoopInfoWrapperPass>();
  AU.addRequired<llvm::DominatorTreeWrapperPass>();
  AU.addPreserved<llvm::DominatorTreeWrapperPass>();
  AU.addRequired<DecoupleLoopsPass>();

  return;
}
} // namespace icsa end
