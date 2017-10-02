//
//
//

#ifndef DECOUPLELOOPSFRONTANNOTATOR_HPP
#define DECOUPLELOOPSFRONTANNOTATOR_HPP

#include "Config.hpp"
#include "DecoupleLoopsFront.hpp"

namespace llvm {
class StringRef;
class BasicBlock;
} // namespace llvm

namespace icsa {
namespace IteratorRecognition {

extern const llvm::StringRef MetadataKey;

void Annotate(const llvm::BasicBlock &BB, Mode M);
bool HasAnnotateMode(const llvm::BasicBlock &BB);
Mode GetAnnotatedMode(const llvm::BasicBlock &BB);

} // namespace IteratorRecognition
} // namespace icsa

#endif // header
