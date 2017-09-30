//
//
//

#ifndef DECOUPLELOOPSFRONT_HPP
#define DECOUPLELOOPSFRONT_HPP

#if DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS
#include "DecoupleLoops.h"
#endif // DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS

namespace llvm {
class Instruction;
class Loop;
} // namespace llvm end

namespace icsa {

enum class IteratorRecognitionMode : unsigned { IteratorMode, PayloadMode };

inline IteratorRecognitionMode InvertMode(IteratorRecognitionMode mode) {
  return mode == IteratorRecognitionMode::PayloadMode
             ? IteratorRecognitionMode::IteratorMode
             : IteratorRecognitionMode::PayloadMode;
}

inline IteratorRecognitionMode GetMode(const llvm::Instruction &Inst,
                                       const llvm::Loop &CurLoop,
                                       const DecoupleLoopsPass &DLP) {
  return DLP.isWork(Inst, &CurLoop) ? IteratorRecognitionMode::PayloadMode
                                    : IteratorRecognitionMode::IteratorMode;
}

} // namespace icsa end

#endif // header
