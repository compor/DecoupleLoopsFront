//
//
//

#ifndef DECOUPLELOOPSFRONT_HPP
#define DECOUPLELOOPSFRONT_HPP

#if DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS
#include "DecoupleLoops.h"
#endif // DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include <map>
// using std::map

#include <vector>
// using std::vector

#include <utility>
// using std::pair

namespace llvm {
class Instruction;
class BasicBlock;
class Loop;
} // namespace llvm end

namespace icsa {

namespace IteratorRecognition {

enum class Mode : unsigned { Iterator, Payload };

template <typename T> using ModeMapTy = std::map<T, Mode>;
template <typename T> using ModeChangePointTy = std::pair<T, Mode>;
template <typename T, typename Y>
using ModeChangePointMapTy = std::map<T, std::vector<ModeChangePointTy<Y>>>;

using BlockModeMapTy = ModeMapTy<llvm::BasicBlock *>;
using BlockModeChangePointTy = ModeChangePointTy<llvm::Instruction *>;
using BlockModeChangePointMapTy =
    ModeChangePointMapTy<llvm::BasicBlock *, llvm::Instruction *>;

} // namespace IteratorRecognition end

inline IteratorRecognition::Mode InvertMode(IteratorRecognition::Mode m) {
  namespace itr = IteratorRecognition;

  return m == itr::Mode::Payload ? itr::Mode::Iterator : itr::Mode::Payload;
}

inline IteratorRecognition::Mode GetMode(const llvm::Instruction &Inst,
                                         const llvm::Loop &CurLoop,
                                         const DecoupleLoopsPass &DLP) {
  namespace itr = IteratorRecognition;

  return DLP.isWork(Inst, &CurLoop) ? itr::Mode::Payload : itr::Mode::Iterator;
}

inline llvm::StringRef GetModePrefix(IteratorRecognition::Mode mode) {
  return mode == IteratorRecognition::Mode::Payload ? "pd_" : "it_";
}

bool FindPartitionPoints(
    const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP,
    IteratorRecognition::BlockModeMapTy &Modes,
    IteratorRecognition::BlockModeChangePointMapTy &Points);

} // namespace icsa end

#endif // header
