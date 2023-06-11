# CMake Test Coverage

CMake GitHub workflow for coverage runs these commands.

```shell
cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="--coverage -Og" \
  -DCMAKE_CXX_FLAGS="--coverage -Og" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage -Og" \
  -S . -B "bld/"

cmake --build "bld/" --config Debug

# Run tests to generate .gcda coverage files.
cd "bld/"
ctest -C Debug
cd ..

# Gather coverage files into one .info file.
lcov --capture --directory "bld/" -o "bld/coverage_report.info"

# Filter out dependencies from coverage info.
lcov --remove "bld/coverage_report.info" -o "bld/coverage_report.info" \
  "/usr/include/*" "${PWD}/bld/_deps/*"

# Show result.
lcov --list "bld/coverage_report.info"
```
