//
//
//

#include "DecoupleLoopsFrontWeights.hpp"
#include "DecoupleLoopsFrontAnnotator.hpp"

#include "llvm/Analysis/LoopInfo.h"
// using llvm::Loop

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSet

#include <cassert>
// using assert

namespace icsa {
namespace IteratorRecognition {

BlockPayloadMapTy calculatePayloadWeight(const llvm::Loop &CurLoop,
                                         const DecoupleLoopsPass *DLP) {
  assert(CurLoop.isLoopSimplifyForm() && "Loop is not in simplify form!");

  llvm::SmallPtrSet<llvm::BasicBlock *, 16> workList;

  std::for_each(CurLoop.block_begin(), CurLoop.block_end(), [&](auto &e) {
    if (!DLP && HasAnnotateMode(*e) &&
        GetAnnotatedMode(*e) == IteratorRecognition::Mode::Payload)
      workList.insert(e);
  });

  workList.erase(CurLoop.getLoopLatch());

  return {};
}

} // namespace IteratorRecognition
} // namespace icsa
