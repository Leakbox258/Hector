#ifndef TOR_PASS_DETAIL_H
#define TOR_PASS_DETAIL_H

#include "mlir/Pass/Pass.h"
#include "TOR/TOR.h"

namespace mlir
{
  template <typename ConcreteDialect>
  void registerDialect(DialectRegistry &registry);

  namespace tor
  {
    class TORDialect;
  }

#define GEN_PASS_DEF_SCFTOTOR
#define GEN_PASS_DEF_TORCHECK
#define GEN_PASS_DEF_TORPIPELINEPARTITION
#define GEN_PASS_DEF_TORSCHEDULE
#define GEN_PASS_DEF_TORSPLIT
#define GEN_PASS_DEF_WIDTHANALYSIS
#include "TOR/Passes.h.inc"
}
#endif //TOR_PASS_DETAIL_H
