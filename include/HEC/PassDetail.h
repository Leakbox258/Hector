#ifndef HEC_PASS_DETAIL_H
#define HEC_PASS_DETAIL_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

namespace mlir
{
  template <typename ConcreteDialect>
  void registerDialect(DialectRegistry &registry);

  namespace hec
  {
    class HECDialect;
  }

  namespace tor
  {
    class TORDialect;
  }

#define GEN_PASS_DEF_HECGEN
#define GEN_PASS_DEF_DUMPCHISEL
#define GEN_PASS_DEF_DYNAMICSCHEDULE
#include "HEC/Passes.h.inc"
}
#endif
