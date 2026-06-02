#ifndef HEC_DIALECT_TD
#define HEC_DIALECT_TD

include "mlir/IR/OpBase.td"

def HECDialect : Dialect {
  let name = "hec";
  let summary = "Types and operations for hec dialect";
  let description = [{
    This dialect defines the 'hec' dialect, which ...
  }];
  let cppNamespace = "::mlir::hec";
  
  let extraClassDeclaration = [{
    /// Register all hec types.
    void registerTypes();
  }];
}

class HECOp<string mnemonic, list<Trait> traits = []> :
    Op<HECDialect, mnemonic, traits> {
  let extraClassDeclaration = [{
    ::llvm::StringRef instanceName() {
      return ::llvm::cast<::mlir::StringAttr>(
                 (*this)->getAttr("instanceName")).getValue();
    }
    ::llvm::StringRef componentName() {
      return ::llvm::cast<::mlir::FlatSymbolRefAttr>(
                 (*this)->getAttr("componentName")).getValue();
    }
    ::mlir::StringAttr primitiveNameAttr() {
      return ::llvm::cast<::mlir::StringAttr>((*this)->getAttr("primitiveName"));
    }
    ::llvm::StringRef primitiveName() { return primitiveNameAttr().getValue(); }
    ::mlir::ArrayAttr portNames() {
      return ::llvm::cast<::mlir::ArrayAttr>((*this)->getAttr("portNames"));
    }
    uint64_t numInPorts() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("numInPorts")).getInt();
    }
    ::llvm::StringRef interfc() {
      return ::llvm::cast<::mlir::StringAttr>(
                 (*this)->getAttr("interfc")).getValue();
    }
    ::llvm::StringRef style() {
      return ::llvm::cast<::mlir::StringAttr>(
                 (*this)->getAttr("style")).getValue();
    }
    bool initial() {
      return ::llvm::cast<::mlir::BoolAttr>((*this)->getAttr("initial"))
          .getValue();
    }
    ::mlir::Value lhs() { return (*this)->getOperand(0); }
    ::mlir::Value rhs() { return (*this)->getOperand(1); }
    ::mlir::Value src() { return (*this)->getOperand(0); }
    ::mlir::Value guard() {
      return (*this)->getNumOperands() ? (*this)->getOperand((*this)->getNumOperands() - 1)
                                       : ::mlir::Value();
    }
    ::mlir::Value cond() { return guard(); }
    ::mlir::Value result() { return (*this)->getResult(0); }
    ::mlir::Value res() { return (*this)->getResult(0); }
    ::mlir::FlatSymbolRefAttr symbol() {
      return ::llvm::cast<::mlir::FlatSymbolRefAttr>((*this)->getAttr("symbol"));
    }
  }];
}



#endif // HEC_DIALECT_TD
