#ifndef HEC_GRAPH_TD
#define HEC_GRAPH_TD

def GraphOp : HECOp<"graph", [
    NoRegionArguments,
    NoTerminator,
    SingleBlock,
    RegionKindInterface,
    HasParent<"ComponentOp">
  ]> {
  let summary = "HEC Graph";
  let description = [{
    The "hec.graph" operation represents a handshake graph.

  }];
  let regions = (region SizedRegion<1> : $bodyRegion);

  let extraClassDeclaration = [{
    /// Implement RegionKindInterface.
    static RegionKind getRegionKind(unsigned index) { return RegionKind::Graph; }
    Block *getBody() { return &getOperation()->getRegion(0).front(); }
    Region &body() { return getBodyRegion(); }

  }];
  let assemblyFormat = "$bodyRegion attr-dict";
  let hasVerifier = 1;
}

#endif // HEC_GRAPH_TD
