//
//
//

#include "Config.hpp"
#include "DecoupleLoopsFront.hpp"

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

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

bool FindPartitionPoints(
    const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP,
    IteratorRecognition::BlockModeMapTy &Modes,
    IteratorRecognition::BlockModeChangePointMapTy &Points) {
  bool found = false;

  if (!DLP.hasWork(&CurLoop))
    return found;

  for (auto bi = CurLoop.block_begin(), be = CurLoop.block_end(); bi != be;
       ++bi) {
    auto *bb = *bi;
    auto *firstI = bb->getFirstNonPHI();
    auto lastSeenMode = GetMode(*firstI, CurLoop, DLP);
    bool hasAllSameModeInstructions = true;

    for (llvm::BasicBlock::iterator ii = firstI, ie = bb->end(); ii != ie;
         ++ii) {
      auto &inst = *ii;
      auto curMode = GetMode(inst, CurLoop, DLP);

      if (lastSeenMode != curMode) {
        found = true;
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

  return found;
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
