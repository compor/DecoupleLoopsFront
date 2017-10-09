//
//
//

#include "DecoupleLoopsFrontTypes.hpp"

#include "DecoupleLoopsFront.hpp"

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/IR/Instructions.h"
// using llvm::BranchInst

#include "llvm/IR/Dominators.h"
// using llvm::DominatorTree

#include "llvm/Analysis/LoopInfo.h"
// using llvm::Loop
// using llvm::LoopInfo

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
// using llvm::SplitBlock

#include <algorithm>
// using std::reverse

namespace icsa {

bool IsSingleMode(const llvm::BasicBlock &BB, const llvm::Loop &CurLoop,
                  const DecoupleLoopsPass &DLP) {
  auto lastSeenMode = GetMode(*BB.begin(), CurLoop, DLP);

  for (const auto &e : BB) {
    auto *br = llvm::dyn_cast<llvm::BranchInst>(&e);
    bool isUncondBr = br && br->isUnconditional();

    if (GetMode(e, CurLoop, DLP) != lastSeenMode && !isUncondBr)
      return false;
  }

  return true;
}

bool IsSingleMode(const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP) {
  auto bi = CurLoop.block_begin();

  if (!IsSingleMode(**bi, CurLoop, DLP))
    return false;

  auto lastSeenMode = GetMode(*(*bi)->begin(), CurLoop, DLP);

  ++bi;
  for (auto be = CurLoop.block_end(); bi != be; ++bi) {
    if (!IsSingleMode(**bi, CurLoop, DLP))
      return false;

    auto curMode = GetMode(*(*bi)->begin(), CurLoop, DLP);
    if (lastSeenMode != curMode)
      return false;

    lastSeenMode = curMode;
  }

  return true;
}

bool FindPartitionPoints(
    const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP,
    IteratorRecognition::BlockModeMapTy &Modes,
    IteratorRecognition::BlockModeChangePointMapTy &Points) {
  if (!DLP.hasWork(&CurLoop))
    return false;

  for (auto bi = CurLoop.block_begin(), be = CurLoop.block_end(); bi != be;
       ++bi) {
    auto *bb = *bi;
    auto *firstI = bb->getFirstNonPHI();
    auto lastSeenMode = GetMode(*firstI, CurLoop, DLP);
    bool hasAllSameModeInstructions = true;

    for (llvm::BasicBlock::iterator ii = firstI, ie = bb->end(); ii != ie;
         ++ii) {
      auto &inst = *ii;
      auto *br = llvm::dyn_cast<llvm::BranchInst>(&inst);
      bool isUncondBr = br && br->isUnconditional();
      auto curMode = GetMode(inst, CurLoop, DLP);

      if (lastSeenMode != curMode && !isUncondBr) {
        hasAllSameModeInstructions = false;
        auto modeChangePt = std::make_pair(&inst, curMode);

        if (Points.find(bb) == Points.end())
          Points.emplace(
              bb, std::vector<IteratorRecognition::BlockModeChangePointTy>{});

        Points.at(bb).push_back(modeChangePt);
        lastSeenMode = curMode;
      }
    }

    if (hasAllSameModeInstructions)
      Modes.emplace(bb, lastSeenMode);
  }

  return true;
}

void SplitAtPartitionPoints(
    IteratorRecognition::BlockModeChangePointMapTy &Points,
    IteratorRecognition::BlockModeMapTy &Modes, llvm::DominatorTree *DT,
    llvm::LoopInfo *LI) {
  for (auto &e : Points) {
    auto *oldBB = e.first;
    IteratorRecognition::Mode lastMode;
    std::reverse(e.second.begin(), e.second.end());

    for (auto &k : e.second) {
      auto *splitI = k.first;
      lastMode = k.second;
      Modes.emplace(llvm::SplitBlock(oldBB, splitI, DT, LI), lastMode);
    }

    Modes.emplace(oldBB, InvertMode(lastMode));
  }

  return;
}

} // namespace icsa
