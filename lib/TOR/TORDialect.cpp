#include "TOR/TORDialect.h"
#include "TOR/TOR.h"
#include "TOR/TORTypes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/Transforms/InliningUtils.h"


using namespace mlir;
using namespace mlir::tor;

#include "TOR/TORDialect.cpp.inc"

struct TORInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  bool isLegalToInline(Operation *call, Operation *callable, bool wouldBeCloned) const final {
    return true;
  }

  void handleTerminator(Operation *op, ValueRange valuesToRepl) const final {
    auto returnOp = cast<tor::ReturnOp>(op);

    assert(returnOp.getNumOperands() == valuesToRepl.size());
    for (const auto &it : llvm::enumerate(returnOp.getOperands()))
      valuesToRepl[it.index()].replaceAllUsesWith(it.value());
  }
};

void TORDialect::initialize()
{
  registerTypes();
  addOperations<
#define GET_OP_LIST
#include "TOR/TOR.cpp.inc"
      >();
  addInterfaces<TORInlinerInterface>();
}

// Provide implementations for the enums we use.
#include "TOR/TOREnums.cpp.inc"
