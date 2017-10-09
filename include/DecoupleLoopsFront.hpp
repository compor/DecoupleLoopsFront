//
//
//

#ifndef DECOUPLELOOPSFRONT_HPP
#define DECOUPLELOOPSFRONT_HPP

#include "Config.hpp"

#include "DecoupleLoopsFrontTypes.hpp"

#if DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS
#include "DecoupleLoops.h"
#endif // DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS

#include "llvm/IR/InstVisitor.h"
// using llvm::InstVisitor

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
class LoopInfo;
class Loop;
class DominatorTree;
} // namespace llvm end

namespace icsa {

namespace IteratorRecognition {

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

bool IsSingleMode(const llvm::BasicBlock &BB, const llvm::Loop &CurLoop,
                  const DecoupleLoopsPass &DLP);

bool IsSingleMode(const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP);

bool FindPartitionPoints(
    const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP,
    IteratorRecognition::BlockModeMapTy &Modes,
    IteratorRecognition::BlockModeChangePointMapTy &Points);

void SplitAtPartitionPoints(
    IteratorRecognition::BlockModeChangePointMapTy &Points,
    IteratorRecognition::BlockModeMapTy &Modes,
    llvm::DominatorTree *DT = nullptr, llvm::LoopInfo *LI = nullptr);

class PayloadPHIChecker : public llvm::InstVisitor<PayloadPHIChecker> {
  const llvm::Loop &m_CurLoop;
  const DecoupleLoopsPass &m_DLP;
  bool m_Status;

public:
  PayloadPHIChecker(const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP)
      : m_Status(false), m_CurLoop(CurLoop), m_DLP(DLP) {}

  // TODO maybe use safe bool idiom instead
  bool getStatus() { return m_Status; }

  void visitPHINode(llvm::PHINode &Inst) {
    if (IteratorRecognition::Mode::Payload != GetMode(Inst, m_CurLoop, m_DLP))
      m_Status = true;
  }
};

} // namespace icsa

#endif // header
