//
//
//

#ifndef DECOUPLELOOPSFRONTANNOTATOR_HPP
#define DECOUPLELOOPSFRONTANNOTATOR_HPP

#include "Config.hpp"
#include "DecoupleLoopsFront.hpp"
#include "DecoupleLoopsFrontWeights.hpp"

namespace llvm {
class StringRef;
class BasicBlock;
} // namespace llvm

namespace icsa {
namespace IteratorRecognition {

extern const llvm::StringRef ModeMetadataKey;
extern const llvm::StringRef PayloadWeightMetadataKey;

void Annotate(llvm::BasicBlock &BB, Mode M);
void Annotate(llvm::BasicBlock &BB, const PayloadWeightTy &W);
bool IsAnnotatedWithMode(const llvm::BasicBlock &BB);
Mode GetAnnotatedMode(const llvm::BasicBlock &BB);

} // namespace IteratorRecognition
} // namespace icsa

#endif // header
