#include "TOR/TOR.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir;
using namespace tor;

static LogicalResult verifyDesignOp(tor::DesignOp op)
{
  // TODO: check the existance of a `main` func
  return success();
}

void tor::AddIOp::build(OpBuilder &odsBuilder,
                         OperationState &odsState,
                         Value lhs,
                         Value rhs) 
{
  IntegerType lhsType = llvm::cast<IntegerType>(lhs.getType());
  IntegerType rhsType = llvm::cast<IntegerType>(rhs.getType());
  IntegerType resType = IntegerType::get(odsState.getContext(), std::max(lhsType.getWidth(), rhsType.getWidth()));
  odsState.addAttribute("starttime", odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), 0));
  odsState.addAttribute("endtime", odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), 0));
  return build(odsBuilder, odsState, resType, ValueRange{lhs, rhs}, ArrayRef<NamedAttribute>{});
}

void tor::SubIOp::build(OpBuilder &odsBuilder,
                         OperationState &odsState,
                         Value lhs,
                         Value rhs) 
{
  IntegerType lhsType = llvm::cast<IntegerType>(lhs.getType());
  IntegerType rhsType = llvm::cast<IntegerType>(rhs.getType());
  IntegerType resType = IntegerType::get(odsState.getContext(), std::max(lhsType.getWidth(), rhsType.getWidth()));
  odsState.addAttribute("starttime", odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), 0));
  odsState.addAttribute("endtime", odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), 0));
  return build(odsBuilder, odsState, resType, ValueRange{lhs, rhs}, ArrayRef<NamedAttribute>{});
}

void tor::MulIOp::build(OpBuilder &odsBuilder,
                         OperationState &odsState,
                         Value lhs,
                         Value rhs) 
{
  IntegerType lhsType = llvm::cast<IntegerType>(lhs.getType());
  IntegerType rhsType = llvm::cast<IntegerType>(rhs.getType());
  IntegerType resType = IntegerType::get(odsState.getContext(), std::min(64U, lhsType.getWidth() + rhsType.getWidth()));

  odsState.addAttribute("starttime", odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), 0));
  odsState.addAttribute("endtime", odsBuilder.getIntegerAttr(odsBuilder.getIntegerType(32), 0));
  return build(odsBuilder, odsState, resType, ValueRange{lhs, rhs}, ArrayRef<NamedAttribute>{});
}

void tor::FuncOp::build(::mlir::OpBuilder &odsBuilder,
                         ::mlir::OperationState &odsState,
                         StringRef name, FunctionType type,
                         ArrayRef<NamedAttribute> attrs,
                         ArrayRef<NamedAttrList> argAttrs)
{
  odsState.addAttribute(SymbolTable::getSymbolAttrName(),
                        odsBuilder.getStringAttr(name));
  odsState.addAttribute(getFunctionTypeAttrName(odsState.name),
                        TypeAttr::get(type));
  odsState.attributes.append(attrs.begin(), attrs.end());
  odsState.addRegion();
}

static void print(::mlir::OpAsmPrinter &p, tor::FuncOp op)
{
  FunctionType funcType = op.getType();
  function_interface_impl::printFunctionOp(
      p, op, /*isVariadic=*/true,
      op.getFunctionTypeAttrName(), {}, {});
  //p.printOptionalAttrDict(op->getAttrs());
}

static ::mlir::ParseResult parseFuncOp(::mlir::OpAsmParser &parser,
                                       ::mlir::OperationState &result)
{
  auto buildFuncType = [](Builder &builder, ArrayRef<Type> argTypes,
                          ArrayRef<Type> results,
                          function_interface_impl::VariadicFlag, std::string &)
  {
    return builder.getFunctionType(argTypes, results);
  };
  if (function_interface_impl::parseFunctionOp(
          parser, result, true, FuncOp::getFunctionTypeAttrName(result.name),
          buildFuncType, {}, {}))
    return failure();
  // Parse the optional attribute list.
  if (parser.parseOptionalAttrDict(result.attributes))
    return failure();
  return success();
}

static ::mlir::LogicalResult verifyFuncOp(tor::FuncOp op)
{
  auto fnInputTypes = op.getType().getInputs();
  Block &entryBlock = op.getBody().front();

  for (unsigned i = 0, e = entryBlock.getNumArguments(); i != e; ++i)
  {
    if (fnInputTypes[i] != entryBlock.getArgument(i).getType())
      return op.emitOpError("type of entry block argument #")
             << i << '(' << entryBlock.getArgument(i).getType()
             << ") must match the type of the corresponding argument in "
             << "module signature(" << fnInputTypes[i] << ')';
  }
  return success();
}

static void print(OpAsmPrinter &p, tor::ReturnOp op)
{
  p << "tor.return";
  if (op.getNumOperands() != 0)
  {
    p << ' ';
    p.printOperands(op.getOperands());
  }
  /*
  p << " at " << op.time();*/
  if (op.getNumOperands() != 0)
  {
    p << " : ";
    interleaveComma(op.getOperandTypes(), p);
  }
}

static ParseResult parseReturnOp(OpAsmParser &parser, OperationState &result)
{
  SmallVector<OpAsmParser::UnresolvedOperand, 1> opInfo;
  SmallVector<Type, 1> types;
  ::mlir::IntegerAttr timeAttr;
  llvm::SMLoc loc = parser.getCurrentLocation();
  return failure(parser.parseOperandList(opInfo) ||
                 /* parser.parseKeyword("at") ||
                 parser.parseAttribute(timeAttr, 
                    parser.getBuilder().getIntegerType(32), 
                    "time", result.attributes) || */
                 (!opInfo.empty() && parser.parseColonTypeList(types)) ||
                 parser.resolveOperands(opInfo, types, loc, result.operands));
}

static LogicalResult verifyReturnOp(tor::ReturnOp op)
{
  return success();
  auto *parent = op->getParentOp();

  StringRef parentName = parent->getName().getStringRef();

  if (parentName == "tor.func")
  {
    auto function = dyn_cast<tor::FuncOp>(parent);
    // if (!function)
    //   return op.emitOpError("must have a handshake.func parent");

    // The operand number and types must match the function signature.
    const auto &results = function.getType().getResults();
    if (op.getNumOperands() != results.size())
      return op.emitOpError("has ")
             << op.getNumOperands()
             << " operands, but enclosing function returns " << results.size();

    for (unsigned i = 0, e = results.size(); i != e; ++i)
      if (op.getOperand(i).getType() != results[i])
        return op.emitError()
               << "type of return operand " << i << " ("
               << op.getOperand(i).getType()
               << ") doesn't match function result type (" << results[i] << ")";

    return success();
  }
  return op.emitOpError("must have a tor.func or tor.module parent");
}

static void print(OpAsmPrinter &p, TimeGraphOp op)
{

  p << TimeGraphOp::getOperationName() << " (" << op.starttime() << " to " << op.endtime() << ")";

  p.printRegion(op.region(),
                /*printEntryBlockArgs=*/false,
                /*printBlockTerminators=*/false);
}

static ParseResult parseTimeGraphOp(OpAsmParser &parser, OperationState &result)
{
  result.regions.reserve(1);
  Region *region = result.addRegion();

  ::mlir::IntegerAttr starttime;
  ::mlir::IntegerAttr endtime;

  if (/*parser.parseKeyword("on") || */ parser.parseLParen() ||
      parser.parseAttribute(starttime,
                            parser.getBuilder().getIntegerType(32),
                            "starttime", result.attributes) ||
      parser.parseKeyword("to") ||
      parser.parseAttribute(endtime,
                            parser.getBuilder().getIntegerType(32),
                            "endtime", result.attributes) ||
      parser.parseRParen())
  {
    return failure();
  }

  if (/*parser.parseKeyword("then") ||*/
      parser.parseRegion(*region))
  {
    return failure();
  }

  TimeGraphOp::ensureTerminator(*region, parser.getBuilder(), result.location);
  return success();
}

static LogicalResult verifyStartTimeOp(tor::StartTimeOp op)
{
  return success();
}

static LogicalResult verifyEndTimeOp(tor::EndTimeOp op)
{
  return success();
}

static LogicalResult verifySuccTimeOp(tor::SuccTimeOp op)
{
  return success();
  // TODO check the equal length
}

static void print(OpAsmPrinter &p, IfOp op)
{
  bool printBlockTerminators = false;

  p << IfOp::getOperationName() << " " << op.condition()
    << " on (" << op.starttime() << " to " << op.endtime() << ")";

  if (!op.results().empty())
  {
    p << " -> (" << op.getResultTypes() << ")";
    printBlockTerminators = true;
  }

  p << " then";

  p.printRegion(op.thenRegion(),
                /*printEntryBlockArgs=*/false,
                /*printBlockTerminators=*/printBlockTerminators);

  auto &elseRegion = op.elseRegion();

  if (!elseRegion.empty())
  {
    p << " else ";
    p.printRegion(elseRegion,
                  /*printEntryBlockArgs=*/false,
                  /*printBlockTerminators=*/printBlockTerminators);
  }

  p.printOptionalAttrDict(op->getAttrs(), {"starttime", "endtime"});
}

static ParseResult parseIfOp(OpAsmParser &parser, OperationState &result)
{
  result.regions.reserve(2);
  Region *thenRegion = result.addRegion();
  Region *elseRegion = result.addRegion();

  ::mlir::IntegerAttr starttime;
  ::mlir::IntegerAttr endtime;

  auto &builder = parser.getBuilder();
  OpAsmParser::UnresolvedOperand cond;

  Type i1Type = builder.getIntegerType(1);
  if (parser.parseOperand(cond) ||
      parser.resolveOperand(cond, i1Type, result.operands))
  {
    return failure();
  }

  if (parser.parseKeyword("on") || parser.parseLParen() ||
      parser.parseAttribute(starttime,
                            parser.getBuilder().getIntegerType(32),
                            "starttime", result.attributes) ||
      parser.parseKeyword("to") ||
      parser.parseAttribute(endtime,
                            parser.getBuilder().getIntegerType(32),
                            "endtime", result.attributes) ||
      parser.parseRParen())
  {
    return failure();
  }
  if (parser.parseOptionalArrowTypeList(result.types))
  {
    return failure();
  }

  if (parser.parseKeyword("then") ||
      parser.parseRegion(*thenRegion))
  {
    return failure();
  }
  IfOp::ensureTerminator(*thenRegion, parser.getBuilder(), result.location);

  if (!parser.parseOptionalKeyword("else"))
  {
    if (parser.parseRegion(*elseRegion))
    {
      return failure();
    }
    IfOp::ensureTerminator(*elseRegion, parser.getBuilder(), result.location);
  }

  // Parse the optional attribute list.
  if (parser.parseOptionalAttrDict(result.attributes))
    return failure();

  return success();
}

static LogicalResult verifyIfOp(IfOp op)
{
  return success();
}

void tor::ForOp::build(OpBuilder &builder, OperationState &result, Value lb,
                        Value ub, Value step,
                        IntegerAttr starttime, IntegerAttr endtime,
                        ValueRange iterArgs,
                        BodyBuilderFn bodyBuilder)
{
  result.addAttribute("starttime", starttime);
  result.addAttribute("endtime", endtime);
  result.addOperands({lb, ub, step});
  result.addOperands(iterArgs);
  for (Value v : iterArgs)
    result.addTypes(v.getType());
  result.addRegion();
}

/// Prints the initialization list in the form of
///   <prefix>(%inner = %outer, %inner2 = %outer2, <...>)
/// where 'inner' values are assumed to be region arguments and 'outer' values
/// are regular SSA values.
static void printInitializationList(OpAsmPrinter &p,
                                    Block::BlockArgListType blocksArgs,
                                    ValueRange initializers,
                                    StringRef prefix = "")
{
  assert(blocksArgs.size() == initializers.size() &&
         "expected same length of arguments and initializers");
  if (initializers.empty())
    return;

  p << prefix << '(';
  llvm::interleaveComma(llvm::zip(blocksArgs, initializers), p, [&](auto it)
                        { p << std::get<0>(it) << " = " << std::get<1>(it); });
  p << ")";
}

static void print(OpAsmPrinter &p, tor::ForOp op)
{
  p << op.getOperationName() << " " << op.getInductionVar() << " = "
    << "(" << op.lowerBound() << " : " << op.lowerBound().getType() << ")"
    << " to " 
    << "(" << op.upperBound() << " : " << op.upperBound().getType() << ")" 
    << " step " << "(" << op.step() << " : " << op.step().getType() << ")";

  p.printNewline();
  p << "on (" << op.starttime() << " to " << op.endtime() << ")";

  printInitializationList(p, op.getRegionIterArgs(), op.getIterOperands(), " iter_args");

  if (!op.getIterOperands().empty())
    p << " -> (" << op.getIterOperands().getTypes() << ")";
  p.printRegion(op.region(),
                /*printEntryBlockArgs=*/false,
                /*printBlockTerminators=*/op.hasIterOperands());
  p.printOptionalAttrDict(op->getAttrs(), {"starttime", "endtime"});
}

static ParseResult parseForOp(OpAsmParser &parser, OperationState &result)
{
  auto &builder = parser.getBuilder();
  OpAsmParser::Argument inductionVariable;
  OpAsmParser::UnresolvedOperand lb, ub, step;
  ::mlir::IntegerAttr starttime, endtime;

  // Parse the induction variable followed by '='.
  if (parser.parseArgument(inductionVariable) || parser.parseEqual())
    return failure();

  // Parse loop bounds.
  //Type indexType = builder.getIndexType();
  Type lbType, ubType, stepType;
  if (parser.parseLParen() || parser.parseOperand(lb) ||
      parser.parseColon() || parser.parseType(lbType) || parser.parseRParen() ||
      parser.resolveOperand(lb, lbType, result.operands))
      return failure();

  if (parser.parseKeyword("to"))
    return failure();
  
  if (parser.parseLParen() || parser.parseOperand(ub) ||
      parser.parseColon() || parser.parseType(ubType) || parser.parseRParen() ||
      parser.resolveOperand(ub, ubType, result.operands))
      return failure();

  if (parser.parseKeyword("step"))
    return failure();

  if (parser.parseLParen() || parser.parseOperand(step) ||
      parser.parseColon() || parser.parseType(stepType) || parser.parseRParen() ||
      parser.resolveOperand(step, stepType, result.operands))
      return failure();
  /*
  Type indexType = builder.getIndexType();
  if (parser.parseOperand(lb) ||
      parser.resolveOperand(lb, indexType, result.operands) ||
      parser.parseKeyword("to") || parser.parseOperand(ub) ||
      parser.resolveOperand(ub, indexType, result.operands) ||
      parser.parseKeyword("step") || parser.parseOperand(step) ||
      parser.resolveOperand(step, indexType, result.operands))
    return failure();
  */
  // Parse "on [starttime, endtime]"
  if (parser.parseKeyword("on") || parser.parseLParen() ||
      parser.parseAttribute(starttime,
                            parser.getBuilder().getIntegerType(32),
                            "starttime", result.attributes) ||
      parser.parseKeyword("to") ||
      parser.parseAttribute(endtime,
                            parser.getBuilder().getIntegerType(32),
                            "endtime", result.attributes) ||
      parser.parseRParen())
    return failure();

  // Parse the optional initial iteration arguments.
  SmallVector<OpAsmParser::Argument, 4> regionArgs;
  SmallVector<OpAsmParser::UnresolvedOperand, 4> operands;
  regionArgs.push_back(inductionVariable);

  if (succeeded(parser.parseOptionalKeyword("iter_args")))
  {
    // Parse assignment list and results type list.
    if (parser.parseAssignmentList(regionArgs, operands) ||
        parser.parseArrowTypeList(result.types))
      return failure();
    // Resolve input operands.
    for (auto operand_type : llvm::zip(operands, result.types))
      if (parser.resolveOperand(std::get<0>(operand_type),
                                std::get<1>(operand_type), result.operands))
        return failure();
  }

  // Induction variable.
  Type iterType = builder.getIntegerType(std::max(lbType.getIntOrFloatBitWidth(), 
      std::max(ubType.getIntOrFloatBitWidth(), stepType.getIntOrFloatBitWidth())));

  regionArgs[0].type = iterType;
  for (auto &&it : llvm::zip(llvm::drop_begin(regionArgs), result.types))
    std::get<0>(it).type = std::get<1>(it);

  // Parse the body region.
  Region *body = result.addRegion();
  if (regionArgs.size() != result.types.size() + 1)
    return parser.emitError(
        parser.getNameLoc(),
        "mismatch in number of loop-carried values and defined values");

  if (parser.parseRegion(*body, regionArgs))
    return failure();

  tor::ForOp::ensureTerminator(*body, builder, result.location);

  // Parse the optional attribute list.
  if (parser.parseOptionalAttrDict(result.attributes))
    return failure();

  return success();
}
static LogicalResult verifyForOp(tor::ForOp op)
{
  return success();
}

static void print(OpAsmPrinter &p, tor::WhileOp op)
{

  p << op.getOperationName();
  printInitializationList(p, op.before().front().getArguments(), op.inits(),
                          " ");
  p.printNewline();
  p << "on (" << op.starttime() << " to " << op.endtime() << ")";
  p << " : ";
  p.printFunctionalType(op.inits().getTypes(), op.results().getTypes());
  p.printRegion(op.before(), /*printEntryBlockArgs=*/false);
  p << " do";
  p.printRegion(op.after());
  p.printOptionalAttrDictWithKeyword(op->getAttrs(), {"starttime", "endtime"});
}

static ParseResult parseWhileOp(OpAsmParser &parser, OperationState &result)
{
  SmallVector<OpAsmParser::Argument, 4> regionArgs;
  SmallVector<OpAsmParser::UnresolvedOperand, 4> operands;
  Region *before = result.addRegion();
  Region *after = result.addRegion();

  OptionalParseResult listResult =
      parser.parseOptionalAssignmentList(regionArgs, operands);
  if (listResult.has_value() && failed(*listResult))
    return failure();

  ::mlir::IntegerAttr starttime, endtime;
  if (parser.parseKeyword("on") || parser.parseLParen() ||
      parser.parseAttribute(starttime,
                            parser.getBuilder().getIntegerType(32),
                            "starttime", result.attributes) ||
      parser.parseKeyword("to") ||
      parser.parseAttribute(endtime,
                            parser.getBuilder().getIntegerType(32),
                            "endtime", result.attributes) ||
      parser.parseRParen())
  {
    return failure();
  }

  FunctionType functionType;
  llvm::SMLoc typeLoc = parser.getCurrentLocation();
  if (failed(parser.parseColonType(functionType)))
    return failure();

  result.addTypes(functionType.getResults());

  if (functionType.getNumInputs() != operands.size())
  {
    return parser.emitError(typeLoc)
           << "expected as many input types as operands "
           << "(expected " << operands.size() << " got "
           << functionType.getNumInputs() << ")";
  }

  // Resolve input operands.
  if (failed(parser.resolveOperands(operands, functionType.getInputs(),
                                    parser.getCurrentLocation(),
                                    result.operands)))
    return failure();

  for (auto &&it : llvm::zip(regionArgs, functionType.getInputs()))
    std::get<0>(it).type = std::get<1>(it);

  return failure(
      parser.parseRegion(*before, regionArgs) ||
      parser.parseKeyword("do") || parser.parseRegion(*after) ||
      parser.parseOptionalAttrDictWithKeyword(result.attributes));
}

/// Verifies that two ranges of types match, i.e. have the same number of
/// entries and that types are pairwise equals. Reports errors on the given
/// operation in case of mismatch.
template <typename OpTy>
static LogicalResult verifyTypeRangesMatch(OpTy op, TypeRange left,
                                           TypeRange right, size_t lbias,
                                           size_t rbias, StringRef message)
{
  if (left.size() + lbias != right.size() + rbias)
    return op.emitOpError("expects the same number of ") << message;

  for (unsigned i = 0, e = left.size(); i + lbias < e; ++i)
  {
    if (left[i + lbias] != right[i + rbias])
    {
      InFlightDiagnostic diag = op.emitOpError("expects the same types for ")
                                << message;
      diag.attachNote() << "for argument " << i << ", found " << left[i + lbias]
                        << " and " << right[i + rbias];
      return diag;
    }
  }

  return success();
}

/// Verifies that the first block of the given `region` is terminated by a
/// CYieldOp. Reports errors on the given operation if it is not the case.
template <typename TerminatorTy>
static TerminatorTy verifyAndGetTerminator(tor::WhileOp op, Region &region,
                                           StringRef errorMessage)
{
  Operation *terminatorOperation = region.front().getTerminator();
  if (auto yield = dyn_cast_or_null<TerminatorTy>(terminatorOperation))
    return yield;

  auto diag = op.emitOpError(errorMessage);
  if (terminatorOperation)
    diag.attachNote(terminatorOperation->getLoc()) << "terminator here";
  return nullptr;
}

static LogicalResult verifyWhileOp(tor::WhileOp op)
{
  // if (failed(RegionBranchOpInterface::verifyTypes(op)))
  //   return failure();

  auto beforeTerminator = verifyAndGetTerminator<tor::ConditionOp>(
      op, op.before(),
      "expects the 'before' region to terminate with 'tor.condition'");
  if (!beforeTerminator)
    return failure();

  TypeRange trailingTerminatorOperands = beforeTerminator.args().getTypes();
  if (failed(verifyTypeRangesMatch(op, trailingTerminatorOperands,
                                   op.after().getArgumentTypes(), 0, 0,
                                   "trailing operands of the 'before' block "
                                   "terminator and 'after' region arguments")))
    return failure();

  if (failed(verifyTypeRangesMatch(
          op, trailingTerminatorOperands, op.getResultTypes(), 0, 0,
          "trailing operands of the 'before' block terminator and op results")))
    return failure();

  auto afterTerminator = verifyAndGetTerminator<tor::YieldOp>(
      op, op.after(),
      "expects the 'after' region to terminate with 'tor.yield'");
  return success(afterTerminator != nullptr);
}

LogicalResult tor::DesignOp::verify() { return verifyDesignOp(*this); }

ParseResult tor::FuncOp::parse(OpAsmParser &parser, OperationState &result)
{
  return parseFuncOp(parser, result);
}

void tor::FuncOp::print(OpAsmPrinter &p) { ::print(p, *this); }

LogicalResult tor::FuncOp::verify() { return verifyFuncOp(*this); }

ParseResult tor::ReturnOp::parse(OpAsmParser &parser, OperationState &result)
{
  return parseReturnOp(parser, result);
}

void tor::ReturnOp::print(OpAsmPrinter &p) { ::print(p, *this); }

LogicalResult tor::ReturnOp::verify() { return verifyReturnOp(*this); }

ParseResult tor::TimeGraphOp::parse(OpAsmParser &parser,
                                    OperationState &result)
{
  return parseTimeGraphOp(parser, result);
}

void tor::TimeGraphOp::print(OpAsmPrinter &p) { ::print(p, *this); }

LogicalResult tor::StartTimeOp::verify() { return verifyStartTimeOp(*this); }

LogicalResult tor::EndTimeOp::verify() { return verifyEndTimeOp(*this); }

LogicalResult tor::SuccTimeOp::verify() { return verifySuccTimeOp(*this); }

ParseResult tor::IfOp::parse(OpAsmParser &parser, OperationState &result)
{
  return parseIfOp(parser, result);
}

void tor::IfOp::print(OpAsmPrinter &p) { ::print(p, *this); }

LogicalResult tor::IfOp::verify() { return verifyIfOp(*this); }

ParseResult tor::ForOp::parse(OpAsmParser &parser, OperationState &result)
{
  return parseForOp(parser, result);
}

void tor::ForOp::print(OpAsmPrinter &p) { ::print(p, *this); }

LogicalResult tor::ForOp::verify() { return verifyForOp(*this); }

ParseResult tor::WhileOp::parse(OpAsmParser &parser, OperationState &result)
{
  return parseWhileOp(parser, result);
}

void tor::WhileOp::print(OpAsmPrinter &p) { ::print(p, *this); }

LogicalResult tor::WhileOp::verify() { return verifyWhileOp(*this); }

#define GET_OP_CLASSES
#include "TOR/TOR.cpp.inc"
