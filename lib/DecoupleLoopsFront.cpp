//
//
//

#include "Config.hpp"
#include "DecoupleLoopsFront.hpp"

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/Analysis/LoopInfo.h"
// using llvm::Loop

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

} // namespace icsa
