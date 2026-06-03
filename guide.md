All cmds have to be run under dir examples/, otherwise compiler can't read from json.

# 1. SCF/TOR input -> scheduled TOR
```shell
../build/bin/hector-opt \
--scf-to-tor --schedule-tor \
input.mlir \
> out.tor.mlir
```
# 2. scheduled TOR -> split schedule
```shell
../build/bin/hector-opt \
--split-schedule \
out.tor.mlir \
> out.split.mlir
```
# 3. split TOR -> HEC
```shell
../build/bin/hector-opt \
--generate-hec \
out.split.mlir \
> out.hec.mlir
```

# 4. HEC -> Chisel
```shell
../build/bin/hector-opt \
--dump-chisel \
out.hec.mlir > out.scala
```

# One-line static pipeline:
```shell
../build/bin/hector-opt \
--scf-to-tor --schedule-tor --split-schedule --generate-hec \
input.mlir \
> out.hec.mlir
```

# Dynamic scheduling flow:
```shell
../build/bin/hector-opt \
--scf-to-tor --schedule-tor --dynamic-schedule -mlir-disable-threading \
input.mlir \
> out.dynamic.hec.mlir
```
