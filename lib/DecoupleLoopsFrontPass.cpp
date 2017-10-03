//
//
//

#include "DecoupleLoopsFront.hpp"
#include "DecoupleLoopsFrontPass.hpp"
#include "DecoupleLoopsFrontAnnotator.hpp"
#include "Utils.hpp"

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
#include "AnnotateLoops.hpp"
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

#include "llvm/Pass.h"
// using llvm::RegisterPass

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
// using llvm::Loop

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/Transforms/Scalar.h"
// using char llvm::LoopInfoSimplifyID

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include "llvm/Support/GraphWriter.h"
// using llvm:WriteGraph

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc
// using llvm::cl::location

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/FileSystem.h"
// using llvm::sys::fs

#include <set>
// using std::set

#include <algorithm>
// using std::for_each
// using std::reverse

#include <string>
// using std::string

#include <system_error>
// using std::error_code

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

static llvm::cl::opt<bool> AnnotateBlocksWithType(
    "dlf-bb-annotate",
    llvm::cl::desc("annotate each basic block with type using metadata"));

static llvm::cl::opt<bool> PrefixBlocksWithType(
    "dlf-bb-prefix", llvm::cl::desc("prefix with type each basic block name"));

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

void WriteGraphFile(const llvm::Function &Func,
                    llvm::StringRef DotFilenameSuffix,
                    llvm::StringRef DotDirectory = ".") {
  auto dotFilename =
      (DotDirectory + "/cfg." + Func.getName() + DotFilenameSuffix + ".dot")
          .str();

  DEBUG_CMD(llvm::errs() << "writing file: " << dotFilename << "\n");
  std::error_code ec;
  llvm::raw_fd_ostream dotFile(dotFilename, ec, llvm::sys::fs::F_Text);

  if (!ec)
    llvm::WriteGraph(dotFile, &Func, llvm::sys::fs::F_Text);
  else
    DEBUG_CMD(llvm::errs() << "error writing file: " << dotFilename << "\n");
}

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
std::set<unsigned int> ModifiedLoops;
std::set<unsigned int> UnmodifiedLoops;
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

using FunctionName_t = std::string;

std::set<FunctionName_t> ModifiedFunctions;

void Report(llvm::StringRef FilenamePrefix) {
  std::error_code err;

  auto filename = FilenamePrefix.str() + "ModifiedFunctions" + ".txt";
  llvm::raw_fd_ostream report1(filename, err, llvm::sys::fs::F_Text);

  if (!err)
    for (const auto &e : ModifiedFunctions)
      report1 << e << "\n";
  else
    llvm::errs() << "could not open file: \"" << filename
                 << "\" reason: " << err.message() << "\n";

  report1.close();

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
  filename = FilenamePrefix.str() + "ModifiedLoops" + ".txt";
  llvm::raw_fd_ostream report2(filename, err, llvm::sys::fs::F_Text);

  if (!err)
    for (const auto &e : ModifiedLoops)
      report2 << e << "\n";
  else
    llvm::errs() << "could not open file: \"" << filename
                 << "\" reason: " << err.message() << "\n";

  report2.close();

  //

  filename = FilenamePrefix.str() + "UnmodifiedLoops" + ".txt";
  llvm::raw_fd_ostream report3(filename, err, llvm::sys::fs::F_Text);

  if (!err)
    for (const auto &e : UnmodifiedLoops)
      report3 << e << "\n";
  else
    llvm::errs() << "could not open file: \"" << filename
                 << "\" reason: " << err.message() << "\n";

  report3.close();
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

  return;
}

} // namespace anonymous end

//

bool DecoupleLoopsFrontPass::runOnModule(llvm::Module &CurMod) {
  IteratorRecognition::BlockModeChangePointMapTy modeChanges;
  IteratorRecognition::BlockModeMapTy blockModes;
  bool hasModuleChanged = false;
  bool hasFunctionChanged = false;
  bool shouldReport = !ReportFilenamePrefix.empty();
  llvm::SmallVector<llvm::Loop *, 16> workList;

  for (auto &CurFunc : CurMod) {
    hasFunctionChanged = false;
    workList.clear();
    modeChanges.clear();
    blockModes.clear();

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
    unsigned lastSeenID = 0;

    auto loopsFilter = [&](auto *e) {
      if (al.hasAnnotatedId(*e))
        workList.push_back(e);
    };
#else
    auto loopsFilter = [&](auto *e) { workList.push_back(e); };
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

    std::for_each(DLPLI.begin(), DLPLI.end(), loopsFilter);
    std::reverse(workList.begin(), workList.end());

    for (auto *e : workList) {
      bool found = FindPartitionPoints(*e, DLP, blockModes, modeChanges);

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
      lastSeenID = al.getAnnotatedId(*e);
      found ? ModifiedLoops.insert(lastSeenID)
            : UnmodifiedLoops.insert(lastSeenID);
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
    }

    // transform part
    if (modeChanges.size()) {
      DEBUG_CMD(llvm::errs() << "transform func: " + CurFunc.getName() + "\n");

      SplitAtPartitionPoints(modeChanges, blockModes, &DT, &LI);

      if (PrefixBlocksWithType)
        for (auto &e : blockModes)
          e.first->setName(GetModePrefix(e.second) + e.first->getName());

      if (AnnotateBlocksWithType)
        for (auto &e : blockModes)
          IteratorRecognition::Annotate(*e.first, e.second);

      if (shouldReport)
        ModifiedFunctions.insert(CurFunc.getName());

      hasModuleChanged = hasFunctionChanged = true;
    }

    if (DotCFGOnly && hasFunctionChanged) {
      llvm::StringRef strId{""};

#if DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS
      if (workList.size() == 1)
        strId = "." + std::to_string(lastSeenID);
#endif // DECOUPLELOOPSFRONT_USES_ANNOTATELOOPS

      WriteGraphFile(CurFunc, strId, DotDirectory);
    }
  }

  if (shouldReport)
    Report(ReportFilenamePrefix);

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
