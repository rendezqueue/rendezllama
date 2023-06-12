
find_path(OpenBLAS_INCLUDE_DIRS
  NAMES "cblas.h"
  PATHS
  "/usr/include/"
  "/usr/include/openblas/"
  "/usr/include/openblas-base/"
  "/usr/local/include/"
  "/usr/local/include/openblas/"
  "/usr/local/include/openblas-base/"
  "/opt/OpenBLAS/include/"
  "$ENV{OpenBLAS_HOME}/"
  "$ENV{OpenBLAS_HOME}/include/"
)
message(STATUS "OpenBLAS_INCLUDE_DIRS: ${OpenBLAS_INCLUDE_DIRS}")

find_library(OpenBLAS_LIBRARIES
  NAMES "openblas"
  PATHS
  "/lib/"
  "/lib/openblas-base/"
  "/lib64/"
  "/usr/lib/"
  "/usr/lib/openblas-base/"
  "/usr/lib64/"
  "/usr/local/lib/"
  "/usr/local/lib64/"
  "/opt/OpenBLAS/lib/"
  "$ENV{OpenBLAS_HOME}/"
  "$ENV{OpenBLAS_HOME}/lib/"
)
message(STATUS "OpenBLAS_LIBRARIES: ${OpenBLAS_LIBRARIES}")

