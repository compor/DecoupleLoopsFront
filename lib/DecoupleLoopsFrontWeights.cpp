//
//
//

#include "DecoupleLoopsFrontWeights.hpp"
#include "DecoupleLoopsFrontAnnotator.hpp"

#include "llvm/Analysis/LoopInfo.h"
// using llvm::Loop

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Instructions.h"
// using llvm::LoadInst
// using llvm::StoreInst
// using llvm::AllocaInst
// using llvm::CastInst
// using llvm::TerminatorInst
// using llvm::GetElementPtrInst

#include "llvm/IR/IntrinsicInst.h"
// using llvm::DbgInfoIntrinsic
// using llvm::MemIntrinsic

#include "llvm/IR/InstVisitor.h"
// using llvm::InstVisitor

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSet

#include <cassert>
// using assert

namespace icsa {
namespace IteratorRecognition {

namespace {

PayloadWeightTy &operator+=(PayloadWeightTy &lhs, const PayloadWeights &rhs) {
  lhs += static_cast<PayloadWeightTy>(rhs);
  return lhs;
}

class PayloadWeightCalculator
    : public llvm::InstVisitor<PayloadWeightCalculator> {
  PayloadWeightTy m_Cost;

public:
  PayloadWeightCalculator() : m_Cost(0) {}

  PayloadWeightTy getWeight() const { return m_Cost; }
  void reset() { m_Cost = 0; }

  void visitLoadInst(llvm::LoadInst &Inst) { m_Cost += PayloadWeights::Memory; }
  void visitCastInst(llvm::CastInst &Inst) { m_Cost += PayloadWeights::Cast; }
  void visitCallInst(llvm::CallInst &Inst) { m_Cost += PayloadWeights::Call; }

  void visitStoreInst(llvm::StoreInst &Inst) {
    m_Cost += PayloadWeights::Memory;
  }

  void visitInstruction(Instruction &Inst) {
    m_Cost += PayloadWeights::Instruction;
  }

  void visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &Inst) {
    m_Cost += PayloadWeights::DebugIntrinsic;
  }

  void visitAllocaInst(llvm::AllocaInst &Inst) {
    m_Cost += PayloadWeights::Memory;
  }

  void visitGetElementPtrInst(llvm::GetElementPtrInst &Inst) {
    m_Cost += PayloadWeights::Memory;
  }

  void visitMemIntrinsic(llvm::MemIntrinsic &Inst) {
    m_Cost += PayloadWeights::Memory;
  }

  void visitTerminatorInst(llvm::TerminatorInst &Inst) {
    auto *br = llvm::dyn_cast<llvm::BranchInst>(&Inst);

    if (br && br->isUnconditional())
      m_Cost += PayloadWeights::Minimum;
    else
      m_Cost += PayloadWeights::Instruction;
  }
};

} // namespace end

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
  workList.erase(CurLoop.getHeader());

  BlockPayloadMapTy blockPayloadMap;
  PayloadWeightCalculator pwc;

  for (auto *e : workList) {
    pwc.reset();
    pwc.visit(*e);
    blockPayloadMap.emplace(e, pwc.getWeight());
  }

  return blockPayloadMap;
}

} // namespace IteratorRecognition
} // namespace icsa
