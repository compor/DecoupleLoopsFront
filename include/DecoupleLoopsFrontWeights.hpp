//
//
//

#ifndef DECOUPLELOOPSFRONTWEIGHTS_HPP
#define DECOUPLELOOPSFRONTWEIGHTS_HPP

namespace icsa {

namespace IteratorRecognition {

using PayloadWeightTy = unsigned;

enum class PayloadWeights : PayloadWeightTy {
  Cast = 1,
  DebugIntrinic = 1,
  Instruction = 2,
  Call = 10,
  IndirectCall = 15,
  Memory = 20,
  MemoryIntrinsics = 20
};

} // namespace IteratorRecognition

} // namespace icsa

#endif // header
