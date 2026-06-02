#ifndef TOR_DIALECT_TD
#define TOR_DIALECT_TD

include "mlir/IR/OpBase.td"

def TORDialect : Dialect {
  let name = "tor";
  let summary = "Types and operations for tor dialect";
  let description = [{
    This dialect defines the 'tor' dialect, which ...
  }];
  let cppNamespace = "::mlir::tor";
  let useDefaultTypePrinterParser = 1;
  
  let extraClassDeclaration = [{
    /// Register all RTL types.
    void registerTypes();
  }];
}

class TOROp<string mnemonic, list<Trait> traits = []> :
    Op<TORDialect, mnemonic, traits> {
  let extraClassDeclaration = [{
    uint32_t starttime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("starttime")).getInt();
    }
    uint32_t endtime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("endtime")).getInt();
    }
    ::mlir::Value lhs() { return (*this)->getOperand(0); }
    ::mlir::Value rhs() { return (*this)->getOperand(1); }
    ::mlir::Value value() { return (*this)->getOperand(0); }
    ::mlir::Value result() { return (*this)->getResult(0); }
    ::mlir::Operation::result_range results() { return (*this)->getResults(); }
    ::mlir::Value in() { return (*this)->getOperand(0); }
    ::mlir::FlatSymbolRefAttr symbol() {
      return ::llvm::cast<::mlir::FlatSymbolRefAttr>((*this)->getAttr("symbol"));
    }
  }];
}



#endif // TOR_DIALECT_TD
