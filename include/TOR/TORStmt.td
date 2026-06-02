def TimeGraphOp : TOROp<"timegraph", [HasParent<"FuncOp">, 
                                       SingleBlockImplicitTerminator<"tor::FinishOp">
                                      /*SingleBlock, NoTerminator*/]> {
  let summary = "bulid time-graph operation";

  let description = [{
    The "tor.timegraph" operation represents a time-graph structure.
    ```
    tor.timegraph (0, 3) {
      ...
    }
    ```
  }];

  let arguments = (ins I32Attr : $starttime,
                       I32Attr : $endtime);
  let regions = (region SizedRegion<1> : $region);

  let extraClassDeclaration = [{
    uint32_t starttime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("starttime")).getInt();
    }
    uint32_t endtime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("endtime")).getInt();
    }
    ::mlir::Region &region() { return getRegion(); }
    ::mlir::Block *getBody() { return &getRegion().front(); }
  }];

  let hasCustomAssemblyFormat = 1;
}

def FinishOp : TOROp<"finish", [Terminator]> {
  let summary = "finish time-graph";
  let description = [{
    The 'tor.finish' operation serves as the terminator of a time graph.
  }];
  let assemblyFormat = "attr-dict";
}

def StartTimeOp : TOROp<"starttime", [/*HasParent<"FuncOp">*/]> {
  let summary = "create start time";
  let description = [{
    The 'tor.starttime' operation represent the starttime of a time graph.
    ```
      tor.starttime 0
    ```
  }];

  let arguments = (ins I32Attr : $starttime);

  let assemblyFormat = "$starttime attr-dict";
  let hasVerifier = 1;
}

def EndTimeOp : TOROp<"endtime", [/*HasParent<"FuncOp">,*/Terminator]> {
  let summary = "declare end time";
  let description = [{
    The 'tor.endtime' operation represent the endtime of a time graph.
    ```
      tor.endtime n
    ```
  }];

  let arguments = (ins I32Attr : $endtime);
  let results = (outs);

  let assemblyFormat = "$endtime attr-dict";

  let hasVerifier = 1;
}


def SuccTimeOp : TOROp<"succ", [HasParent<"TimeGraphOp">]> {
  let summary = "link a time-point";
  let description = [{
    The "tor.succ" operation represents a new time-point linking to the current
    time graph.
    ```
      %t2 = tor.succ 
    ```
  }];

  let arguments = (ins I32Attr: $time, 
                       ArrayAttr : $points, 
                       ArrayAttr : $edges);

  let extraClassDeclaration = [{
    uint32_t time() { return getTime(); }
    ::mlir::ArrayAttr points() { return getPoints(); }
    ::mlir::ArrayAttr edges() { return getEdges(); }
    void timeAttr(::mlir::IntegerAttr attr) { setTimeAttr(attr); }
    void pointsAttr(::mlir::ArrayAttr attr) { setPointsAttr(attr); }
    void edgesAttr(::mlir::ArrayAttr attr) { setEdgesAttr(attr); }
  }];

  let assemblyFormat = [{$time `:` $points $edges attr-dict}];
  let hasVerifier = 1;
}

def IfOp : TOROp<"if", [/*DeclareOpInterfaceMethods<RegionBranchOpInterface>,*/
                         SingleBlockImplicitTerminator<"tor::YieldOp">,
                         RecursiveMemoryEffects,
                         NoRegionArguments]> {
  let summary = "if-then-else operation";

  let description = [{
    The "tor.if" operation represents an conditional branch structure.
    ```
    tor.if %cond0 on (2 to 10)
    then {
      // ... ops ignored
    }
    else {
      // ... ops ignored
    }
    ```
  }];

  let arguments = (ins I1 : $condition, 
                       I32Attr : $starttime,
                       I32Attr : $endtime);
  let results = (outs Variadic<AnyType> : $results);
  let regions = (region SizedRegion<1> : $thenRegion, AnyRegion : $elseRegion);

  let extraClassDeclaration = [{
    ::mlir::Value condition() { return getCondition(); }
    ::mlir::Operation::result_range results() { return getResults(); }
    uint32_t starttime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("starttime")).getInt();
    }
    uint32_t endtime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("endtime")).getInt();
    }
    ::mlir::Region &thenRegion() { return getThenRegion(); }
    ::mlir::Region &elseRegion() { return getElseRegion(); }
    ::mlir::Block *getBody(unsigned index) {
      return &getOperation()->getRegion(index).front();
    }
  }];

  let hasCustomAssemblyFormat = 1;
  let hasVerifier = 1;
}

def ForOp : TOROp<"for", [SingleBlockImplicitTerminator<"tor::YieldOp">,
                           RecursiveMemoryEffects]> {
  let summary = "for operation";
  let description = [{
    The "tor.for" operation represents a loop take 3 SSA value as operands
    that represent the lower bound, upper bound and step respectively.  The
    operation defines an SSA value for its induction variable. It has one
    region capturing the loop body. The induction variable is represented as an
    argument of this region. This SSA value always has type index, which is the
    size of the machine word. The step is a value of type index, required to be
    positive.
    The lower and upper bounds specify a half-open range: the range includes
    the lower bound but does not include the upper bound.

    The body region must contain exactly one block that terminates with
    "scf.yield".

    ```mlir
    tor.for %iv = %lb to %ub step %step on [0, 2] {
      ... // body
    }
    ```
    ```mlir
    %sum_0 = constant 0.0 : f32
    // iter_args binds initial values to the loop's region arguments.
    %sum = scf.for %iv = %lb to %ub step %step on (l to r)
        iter_args(%sum_iter = %sum_0) -> (f32) {
      %t = load %buffer[%iv] : memref<1024xf32>
      %sum_next = addf %sum_iter, %t : f32
      // Yield current iteration sum to next iteration %sum_iter or to %sum
      // if final iteration.
      scf.yield %sum_next : f32
    }
    ```
  }];

  let arguments = (ins SignlessIntegerLike: $lowerBound,
                       SignlessIntegerLike: $upperBound,
                       SignlessIntegerLike: $step,
                       I32Attr : $starttime,
                       I32Attr : $endtime,
                       Variadic<AnyType> : $initArgs);

  let results = (outs Variadic<AnyType>: $results);
  let regions = (region SizedRegion<1> : $region);
  
  let skipDefaultBuilders = 1;
  let builders = [
    OpBuilder<(ins "Value" : $lowerBound, "Value" : $upperBound, "Value" : $step,
      "IntegerAttr" : $starttime, "IntegerAttr" : $endtime,
      CArg<"ValueRange", "{}"> : $initArgs,
      CArg<"function_ref<void(OpBuilder &, Location, Value, ValueRange)>", 
        "nullptr">)>
  ];
  
  let hasCustomAssemblyFormat = 1;
  let hasVerifier = 1;

  let extraClassDeclaration = [{
    using BodyBuilderFn = function_ref<void(OpBuilder &, Location, Value, ValueRange)>;
    unsigned getNumInductionVars() { return 1; }
    unsigned getNumControlOperands() { return 3; }
    ::mlir::Region &region() { return getRegion(); }
    ::mlir::Block *getBody() { return &getRegion().front(); }
    ::mlir::Value lowerBound() { return getLowerBound(); }
    ::mlir::Value upperBound() { return getUpperBound(); }
    ::mlir::Value step() { return getStep(); }
    uint32_t starttime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("starttime")).getInt();
    }
    uint32_t endtime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("endtime")).getInt();
    }
    ::mlir::Operation::operand_range initArgs() { return getInitArgs(); }
    ::mlir::Operation::result_range results() { return getResults(); }
    Value getInductionVar() { return getBody()->getArgument(0); }
    Block::BlockArgListType getRegionIterArgs() {
      return getBody()->getArguments().drop_front(getNumInductionVars());
    }
    Operation::operand_range getIterOperands() {
      return getOperands().drop_front(getNumControlOperands());
    }
    MutableArrayRef<OpOperand> getIterOpOperands() {
      return
        getOperation()->getOpOperands().drop_front(getNumControlOperands());
    }
    bool hasIterOperands() {
      return getOperation()->getNumOperands() > getNumControlOperands();
    }
  }];
}   

def WhileOp : TOROp<"while", 
    [/*DeclareOpInterfaceMethods<RegionBranchOpInterface>,*/
     RecursiveMemoryEffects]> {
  let summary = "while operation"; 
  let description = [{
    The "tor.while" operation represents a general loop structure;

    ```
    tor.while %cond=%x on (4 to 9) -> f32
		do {
      ...
      tor.condition(%cond) %y : f32
    }
    ```

    TODO : support loop pipeline
  }];

  let arguments = (ins Variadic<AnyType> : $inits,
                       I32Attr : $starttime,
                       I32Attr : $endtime);
  let results = (outs Variadic<AnyType> : $results);
  let regions = (region SizedRegion<1> : $before, SizedRegion<1> : $after);

  let extraClassDeclaration = [{
    ::mlir::Operation::operand_range inits() { return getInits(); }
    ::mlir::Operation::result_range results() { return getResults(); }
    uint32_t starttime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("starttime")).getInt();
    }
    uint32_t endtime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("endtime")).getInt();
    }
    ::mlir::Region &before() { return getBefore(); }
    ::mlir::Region &after() { return getAfter(); }
  }];

  let hasCustomAssemblyFormat = 1;
  let hasVerifier = 1;
}


def ConditionOp : TOROp<"condition",
                         [HasParent<"WhileOp">, Pure, Terminator]> {
  let summary = "loop continuation condition";
  let description = [{
    This operation accepts the continuation (i.e., inverse of exit) condition
    of the `scf.while` construct. If its first argument is true, the "after"
    region of `scf.while` is executed, with the remaining arguments forwarded
    to the entry block of the region. Otherwise, the loop terminates.
  }];

  let arguments = (ins I1:$condition, Variadic<AnyType>:$args);

  let extraClassDeclaration = [{
    ::mlir::Value condition() { return getCondition(); }
    ::mlir::Operation::operand_range args() { return getArgs(); }
  }];

  let assemblyFormat =
      [{ `(` $condition `)` attr-dict ($args^ `:` type($args))? }];

}

def YieldOp : TOROp<"yield", [Pure, ReturnLike, Terminator,
                               ParentOneOf<["IfOp", "WhileOp", "ForOp"]>]> {
  let summary = "yield operation";
  let description = [{
    The "tor.yield" operation represents a value passing in if/loop structure.
  }];

  let arguments = (ins Variadic<AnyType> : $results);

  let extraClassDeclaration = [{
    ::mlir::Operation::operand_range results() { return getResults(); }
  }];

  let builders = [OpBuilder<(ins), [{ /* nothing to do */ }]>];
  let assemblyFormat = "attr-dict ($results^ `:` type($results))?";
}


def DesignOp : TOROp<"design", [
    NoRegionArguments,
    NoTerminator,
    SingleBlock,
    IsolatedFromAbove,
    SymbolTable
  ]> {
  let summary = "TOR Design";
  let description = [{
    The "tor.design" operation represents a top-level tor design,
    containing a list of TOR func. This must include a "main" func.
  }];
  let arguments = (ins FlatSymbolRefAttr : $symbol);
  let regions = (region SizedRegion<1>: $bodyRegion);

  let extraClassDeclaration = [{
    // return the body of a tor design.
    Block *getBody() {
      return &getOperation()->getRegion(0).front();
    }
    Region &body() { return getBodyRegion(); }
    ::mlir::FlatSymbolRefAttr symbol() {
      return ::llvm::cast<::mlir::FlatSymbolRefAttr>((*this)->getAttr("symbol"));
    }
    
  }];

  let assemblyFormat = "$symbol $bodyRegion attr-dict";
  let hasVerifier = 1;
}

def FuncOp: TOROp<"func", [
    /*IsolatedFromAbove, */FunctionOpInterface, Symbol, RegionKindInterface,
    CallableOpInterface,
    HasParent<"DesignOp">
    /*,
       SingleBlockImplicitTerminator<"ReturnOp">, HasParent<"DesignOp">*/]>{
  let summary = "TOR Func: func";
  let description = [{
    The "tor.func" operation represents a module under mixed scheduling 
    mode, including a given name, a list of ports, and a body that describes the
    elastic circuit. This is almost exactly like a standard FuncOp, except that 
    it has some extra verification conditions, such as 'each value must only
    have a single use'.
  }];
  let arguments = (ins TypeAttrOf<FunctionType>:$function_type);
  let results = (outs);
  let regions = (region SizedRegion<1>:$body);

  let skipDefaultBuilders = 1;
  let builders = [
    OpBuilder<(ins "StringRef":$name, "FunctionType":$type,
                   CArg<"ArrayRef<NamedAttribute>", "{}">:$attrs,
                   CArg<"ArrayRef<NamedAttrList>", "{}">:$argAttrs)>
  ];

  let hasCustomAssemblyFormat = 1;
  let hasVerifier = 1;

  let extraClassDeclaration = [{
    static RegionKind getRegionKind(unsigned index) { 
      return RegionKind::SSACFG;
    }
    
    Region &body() { return getBody(); }
    Block *getBodyBlock() { return &getBody().front();}
    
    FunctionType getType() { return getFunctionType(); }
    Type cloneTypeWith(TypeRange inputs, TypeRange results) {
      return FunctionType::get((*this)->getContext(), inputs, results);
    }
    ArrayRef<Type> getArgumentTypes() {
      return getType().getInputs();
    }
    ArrayRef<Type> getResultTypes() {
      return getType().getResults();
    }

    Region *getCallableRegion() {
      return isExternal() ? nullptr : &getBody();
    }
    ArrayAttr getArgAttrsAttr() {
      return (*this)->getAttrOfType<ArrayAttr>("arg_attrs");
    }
    ArrayAttr getResAttrsAttr() {
      return (*this)->getAttrOfType<ArrayAttr>("res_attrs");
    }
    void setArgAttrsAttr(ArrayAttr attrs) {
      (*this)->setAttr("arg_attrs", attrs);
    }
    void setResAttrsAttr(ArrayAttr attrs) {
      (*this)->setAttr("res_attrs", attrs);
    }
    Attribute removeArgAttrsAttr() {
      return (*this)->removeAttr("arg_attrs");
    }
    Attribute removeResAttrsAttr() {
      return (*this)->removeAttr("res_attrs");
    }

  }];

}

def ReturnOp : TOROp<"return", [Terminator, ParentOneOf<["FuncOp"/*, "ModuleOp"*/]>,
                               Pure, ReturnLike]> {
  let summary = "tor func terminator return";
  let description = [{
    The "tor.return" operation represents the termination of a TOR func.
  }];

  let arguments = (ins Variadic<AnyType> : $operands/*,
                       I32Attr : $time*/);
  
  let hasCustomAssemblyFormat = 1;
  let hasVerifier = 1;
}

def CallOp : TOROp<"call", [CallOpInterface/*,
    DeclareOpInterfaceMethods<SymbolUserOpInterface>*/]> {
  let summary = "tor call operation";
  let description = [{
    The "tor.call" operation represents a direct call to a tor.func
  }];

  let arguments = (ins
      FlatSymbolRefAttr:$callee,
      I32Attr : $starttime, 
      I32Attr : $endtime,
      Variadic<AnyType>:$operands);
  let results = (outs Variadic<AnyType>);

  let extraClassDeclaration = [{
    StringRef callee() { return getCallee(); }
    uint32_t starttime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("starttime")).getInt();
    }
    uint32_t endtime() {
      return ::llvm::cast<::mlir::IntegerAttr>(
                 (*this)->getAttr("endtime")).getInt();
    }
    FunctionType getCalleeType();
    void setCalleeFromCallable(CallInterfaceCallable callee) {
      (*this)->setAttr("callee", callee.dyn_cast<SymbolRefAttr>());
    }

    /// Get the argument operands to the called function.
    operand_range getArgOperands() {
      return {arg_operand_begin(), arg_operand_end()};
    }
    MutableOperandRange getArgOperandsMutable() {
      return MutableOperandRange(*this, 0, getOperands().size());
    }

    operand_iterator arg_operand_begin() { return operand_begin(); }
    operand_iterator arg_operand_end() { return operand_end(); }

    /// Return the callee of this operation.
    CallInterfaceCallable getCallableForCallee() {
      return (*this)->getAttrOfType<SymbolRefAttr>("callee");
    }
    ArrayAttr getArgAttrsAttr() {
      return (*this)->getAttrOfType<ArrayAttr>("arg_attrs");
    }
    ArrayAttr getResAttrsAttr() {
      return (*this)->getAttrOfType<ArrayAttr>("res_attrs");
    }
    void setArgAttrsAttr(ArrayAttr attrs) {
      (*this)->setAttr("arg_attrs", attrs);
    }
    void setResAttrsAttr(ArrayAttr attrs) {
      (*this)->setAttr("res_attrs", attrs);
    }
    Attribute removeArgAttrsAttr() {
      return (*this)->removeAttr("arg_attrs");
    }
    Attribute removeResAttrsAttr() {
      return (*this)->removeAttr("res_attrs");
    }
  }];

  let assemblyFormat = [{
    $callee `(` $operands `)` `on` ` ` `(` $starttime `to` $endtime `)` attr-dict 
    `:` functional-type($operands, results)
  }];
}
