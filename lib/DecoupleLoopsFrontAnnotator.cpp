//
//
//

#include "DecoupleLoopsFrontAnnotator.hpp"

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/IR/Metadata.h"
// using llvm::MDNode
// using llvm::MDString

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include <cassert>
// using assert

namespace icsa {
namespace IteratorRecognition {

namespace {
constexpr const char *const strIteratorMode = "it";
constexpr const char *const strPayloadMode = "pd";
} // namespace end

const llvm::StringRef MetadataKey = "icsa.itr.mode";

void Annotate(llvm::BasicBlock &BB, Mode M) {
  auto &ctx = BB.getContext();
  auto *term = BB.getTerminator();
  llvm::StringRef strMode =
      Mode::Iterator == M ? strIteratorMode : strPayloadMode;
  auto *node = llvm::MDNode::get(ctx, llvm::MDString::get(ctx, strMode));
  term->setMetadata(MetadataKey, node);
}

bool HasAnnotateMode(const llvm::BasicBlock &BB) {
  return BB.getTerminator()->getMetadata(MetadataKey) ? true : false;
}

Mode GetAnnotatedMode(const llvm::BasicBlock &BB) {
  assert(HasAnnotateMode(BB) && "Terminator does not have required metadata!");

  auto *term = BB.getTerminator();
  auto strMode =
      llvm::cast<llvm::MDString>(term->getMetadata(MetadataKey)->getOperand(0))
          ->getString();

  if (strIteratorMode == strMode)
    return Mode::Iterator;
  else if (strPayloadMode == strMode)
    return Mode::Payload;
  else
    assert(false && "No matching mode could be found!");
}

} // namespace IteratorRecognition
} // namespace icsa
