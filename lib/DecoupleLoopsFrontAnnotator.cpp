//
//
//

#include "DecoupleLoopsFrontAnnotator.hpp"

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/IR/Metadata.h"
// using llvm::Metadata
// using llvm::MDNode
// using llvm::MDTuple
// using llvm::MDString
// using llvm::ConstantAsMetadata

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

namespace icsa {
namespace IteratorRecognition {

const llvm::StringRef MetadataKey = "icsa.itr.mode";

void Annotate(const llvm::BasicBlock &BB, Mode M) {}

bool HasAnnotateMode(const llvm::BasicBlock &BB) { return false; }

Mode GetAnnotatedMode(const llvm::BasicBlock &BB) { return Mode::Iterator; }

} // namespace IteratorRecognition
} // namespace icsa
