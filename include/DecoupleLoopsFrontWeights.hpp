//
//
//

#ifndef DECOUPLELOOPSFRONTWEIGHTS_HPP
#define DECOUPLELOOPSFRONTWEIGHTS_HPP

#include "Config.hpp"

#if DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS
#include "DecoupleLoops.h"
#endif // DECOUPLELOOPSFRONT_USES_DECOUPLELOOPS

#include <map>
// using std::map

#include <limits>
// using std::numeric_limits::min
// using std::numeric_limits::max

namespace llvm {
class BasicBlock;
class Loop;
} // namespace llvm

namespace icsa {
namespace IteratorRecognition {

using PayloadWeightTy = unsigned;

using BlockPayloadMapTy = std::map<llvm::BasicBlock *, PayloadWeightTy>;

enum class PayloadWeights : PayloadWeightTy {
  Minimum = std::numeric_limits<PayloadWeightTy>::min(),
  Cast = 1,
  DebugIntrinsic = 1,
  Instruction = 2,
  Memory = 20,
  Call = 35,
  Maximum = std::numeric_limits<PayloadWeightTy>::max()
};

BlockPayloadMapTy
CalculatePayloadWeight(const llvm::Loop &CurLoop,
                       const DecoupleLoopsPass *DLP = nullptr);

} // namespace IteratorRecognition
} // namespace icsa

#endif // header
