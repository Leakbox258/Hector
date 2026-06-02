#ifndef TOR_OPS_H
#define TOR_OPS_H

#include "TOR/TORDialect.h"
#include "TOR/TORTypes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/RegionKindInterface.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace mlir {
using AddFOp = arith::AddFOp;
using AddIOp = arith::AddIOp;
using AndOp = arith::AndIOp;
using CallOp = func::CallOp;
using CmpFOp = arith::CmpFOp;
using CmpIOp = arith::CmpIOp;
using ConstantOp = arith::ConstantOp;
using ConstantFloatOp = arith::ConstantFloatOp;
using ConstantIntOp = arith::ConstantIntOp;
using DivFOp = arith::DivFOp;
using FPToSIOp = arith::FPToSIOp;
using IndexCastOp = arith::IndexCastOp;
using MulFOp = arith::MulFOp;
using MulIOp = arith::MulIOp;
using NegFOp = arith::NegFOp;
using OrOp = arith::OrIOp;
using SIToFPOp = arith::SIToFPOp;
using SelectOp = arith::SelectOp;
using ShiftLeftOp = arith::ShLIOp;
using SignedDivIOp = arith::DivSIOp;
using SignedShiftRightOp = arith::ShRSIOp;
using SignExtendIOp = arith::ExtSIOp;
using SubFOp = arith::SubFOp;
using SubIOp = arith::SubIOp;
using TruncateIOp = arith::TruncIOp;
using XOrOp = arith::XOrIOp;
namespace tor {

} // namespace tor
} // namespace mlir

#define GET_OP_CLASSES
#include "TOR/TOR.h.inc"


#endif // TOR_OPS_H
