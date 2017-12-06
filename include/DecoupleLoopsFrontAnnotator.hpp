//
//
//

#ifndef DECOUPLELOOPSFRONTANNOTATOR_HPP
#define DECOUPLELOOPSFRONTANNOTATOR_HPP

#include "Config.hpp"
#include "DecoupleLoopsFrontTypes.hpp"
#include "DecoupleLoopsFront.hpp"
#include "DecoupleLoopsFrontWeights.hpp"

#if DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS
#include "DecoupleLoops.h"
#endif // DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS

namespace llvm {
class StringRef;
class Instruction;
class BasicBlock;
class Loop;
} // namespace llvm

namespace icsa {
namespace IteratorRecognition {

extern const llvm::StringRef ModeMetadataKey;
extern const llvm::StringRef PayloadWeightMetadataKey;

void Annotate(llvm::Instruction &Inst, Mode M);
void Annotate(llvm::BasicBlock &BB, Mode M);
void Annotate(llvm::BasicBlock &BB, const PayloadWeightTy &W);
bool IsAnnotatedWithMode(const llvm::Instruction &Inst);
bool IsAnnotatedWithMode(const llvm::BasicBlock &BB);
bool IsAnnotatedWithPayloadWeight(const llvm::BasicBlock &BB);
Mode GetAnnotatedMode(const llvm::Instruction &Inst);
Mode GetAnnotatedMode(const llvm::BasicBlock &BB);
PayloadWeightTy GetAnnotatedPayloadWeight(const llvm::BasicBlock &BB);

class PHIAnnotator : public llvm::InstVisitor<PHIAnnotator> {
  const llvm::Loop &m_CurLoop;
  const DecoupleLoopsPass &m_DLP;

public:
  PHIAnnotator(const llvm::Loop &CurLoop, const DecoupleLoopsPass &DLP)
      : m_CurLoop(CurLoop), m_DLP(DLP) {}

  void visitPHINode(llvm::PHINode &Inst);
};

} // namespace IteratorRecognition
} // namespace icsa

#endif // header
