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
  PayloadWeightTy m_Weight;

public:
  PayloadWeightCalculator() : m_Weight(0) {}

  PayloadWeightTy getWeight() const { return m_Weight; }
  void reset() { m_Weight = 0; }

  void visitLoadInst(llvm::LoadInst &Inst) {
    m_Weight += PayloadWeights::Memory;
  }

  void visitCastInst(llvm::CastInst &Inst) { m_Weight += PayloadWeights::Cast; }

  void visitCallInst(llvm::CallInst &Inst) { m_Weight += PayloadWeights::Call; }

  void visitStoreInst(llvm::StoreInst &Inst) {
    m_Weight += PayloadWeights::Memory;
  }

  void visitInstruction(Instruction &Inst) {
    m_Weight += PayloadWeights::Instruction;
  }

  void visitDbgInfoIntrinsic(llvm::DbgInfoIntrinsic &Inst) {
    m_Weight += PayloadWeights::DebugIntrinsic;
  }

  void visitAllocaInst(llvm::AllocaInst &Inst) {
    m_Weight += PayloadWeights::Memory;
  }

  void visitGetElementPtrInst(llvm::GetElementPtrInst &Inst) {
    m_Weight += PayloadWeights::Memory;
  }

  void visitMemIntrinsic(llvm::MemIntrinsic &Inst) {
    m_Weight += PayloadWeights::Memory;
  }

  void visitTerminatorInst(llvm::TerminatorInst &Inst) {
    auto *br = llvm::dyn_cast<llvm::BranchInst>(&Inst);

    if (br && br->isUnconditional())
      m_Weight += PayloadWeights::Minimum;
    else
      m_Weight += PayloadWeights::Instruction;
  }
};

} // namespace end

BlockPayloadMapTy CalculatePayloadWeight(const llvm::Loop &CurLoop,
                                         const DecoupleLoopsPass *DLP) {
  assert(CurLoop.isLoopSimplifyForm() && "Loop is not in simplify form!");

  llvm::SmallPtrSet<llvm::BasicBlock *, 16> workList;

  if (DLP)
    std::for_each(CurLoop.block_begin(), CurLoop.block_end(), [&](auto &e) {
      assert(IsSingleMode(*e, CurLoop, *DLP) &&
             "Loop basic block is not a single mode!");

      if (GetMode(*e->begin(), CurLoop, *DLP) ==
          IteratorRecognition::Mode::Payload)
        workList.insert(e);
    });
  else
    std::for_each(CurLoop.block_begin(), CurLoop.block_end(), [&](auto &e) {
      if (IsAnnotatedWithMode(*e) &&
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
