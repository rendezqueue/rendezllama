# Development

All commands are to be run from the project's toplevel directory.

## Build
To build the project with `cmake`, run:
```shell
cmake -B bld
cmake --build bld --config RelOnHost
```

## Test
To run the test suite:
```shell
ctest --test-dir bld --timeout 10
```
