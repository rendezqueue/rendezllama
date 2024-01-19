# OpenBLAS Presence Test

By default, we build llama.cpp with OpenBLAS, but we hook it up ourselves and could easily miss something.

```shell
# New shell that dies if a command fails.
sh
set -e

make LLAMA_OPENBLAS_ON=1

# Test that we are defining GGML_USE_OPENBLAS.
grep -e "-DGGML_USE_OPENBLAS" "bld/compile_commands.json"

# Test that ggml.c still uses the compile definition.
grep -e "GGML_USE_OPENBLAS" "bld/_deps/llamacpp-src/ggml.c"
```
