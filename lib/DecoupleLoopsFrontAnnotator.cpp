//
//
//

#include "DecoupleLoopsFrontAnnotator.hpp"
#include "DecoupleLoopsFrontWeights.hpp"

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/IR/Type.h"
// using llvm::IntType

#include "llvm/IR/Constants.h"
// using llvm::ConstantInt

#include "llvm/IR/Metadata.h"
// using llvm::MDNode
// using llvm::MDString
// using llvm::ConstantAsMetadata

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

const llvm::StringRef ModeMetadataKey = "icsa.itr.mode";
const llvm::StringRef PayloadWeightMetadataKey = "icsa.itr.payload.weight";

void Annotate(llvm::Instruction &Inst, Mode M) {
  auto &ctx = Inst.getParent()->getContext();
  llvm::StringRef strMode =
      Mode::Iterator == M ? strIteratorMode : strPayloadMode;
  auto *node = llvm::MDNode::get(ctx, llvm::MDString::get(ctx, strMode));
  Inst.setMetadata(ModeMetadataKey, node);
}

void Annotate(llvm::BasicBlock &BB, Mode M) {
  auto &ctx = BB.getContext();
  auto *term = BB.getTerminator();
  llvm::StringRef strMode =
      Mode::Iterator == M ? strIteratorMode : strPayloadMode;
  auto *node = llvm::MDNode::get(ctx, llvm::MDString::get(ctx, strMode));
  term->setMetadata(ModeMetadataKey, node);
}

void Annotate(llvm::BasicBlock &BB, const PayloadWeightTy &W) {
  assert(IsAnnotatedWithMode(BB) &&
         "Basic block is not annotated with mode metadata!");

  assert(GetAnnotatedMode(BB) == Mode::Payload &&
         "Basic block is not annotated with payload mode metadata!");

  auto &ctx = BB.getContext();
  auto *term = BB.getTerminator();

  auto *intType = llvm::Type::getInt32Ty(ctx);
  auto *constantW = llvm::ConstantInt::get(intType, W);

  auto *node = llvm::MDNode::get(ctx, llvm::ConstantAsMetadata::get(constantW));
  term->setMetadata(PayloadWeightMetadataKey, node);
}

bool IsAnnotatedWithMode(const llvm::BasicBlock &BB) {
  return BB.getTerminator()->getMetadata(ModeMetadataKey) ? true : false;
}

bool IsAnnotatedWithPayloadWeight(const llvm::BasicBlock &BB) {
  return BB.getTerminator()->getMetadata(PayloadWeightMetadataKey) ? true
                                                                   : false;
}

Mode GetAnnotatedMode(const llvm::BasicBlock &BB) {
  assert(IsAnnotatedWithMode(BB) &&
         "Terminator does not have required metadata!");

  auto *term = BB.getTerminator();
  auto strMode = llvm::cast<llvm::MDString>(
                     term->getMetadata(ModeMetadataKey)->getOperand(0))
                     ->getString();

  if (strIteratorMode == strMode)
    return Mode::Iterator;
  else if (strPayloadMode == strMode)
    return Mode::Payload;
  else
    assert(false && "No matching mode could be found!");
}

PayloadWeightTy GetAnnotatedPayloadWeight(const llvm::BasicBlock &BB) {
  assert(IsAnnotatedWithMode(BB) &&
         "Basic block is not annotated with mode metadata!");

  assert(IsAnnotatedWithPayloadWeight(BB) &&
         "Terminator does not have required metadata!");

  auto *term = BB.getTerminator();
  auto mdW = llvm::cast<llvm::ConstantAsMetadata>(
      term->getMetadata(ModeMetadataKey)->getOperand(0));

  return mdW->getValue()->getUniqueInteger().getLimitedValue();
}

void PayloadPHIAnnotator::visitPHINode(llvm::PHINode &Inst) {
  if (IteratorRecognition::Mode::Payload != GetMode(Inst, m_CurLoop, m_DLP))
    Annotate(Inst, IteratorRecognition::Mode::Iterator);
}

} // namespace IteratorRecognition
} // namespace icsa
