cmake_minimum_required (VERSION 3.10)
project (pxx)

#
# Output build info.
#

message(STATUS "CMAKE_SYSTEM_NAME          = ${CMAKE_SYSTEM_NAME}"         )
message(STATUS "CMAKE_CXX_COMPILER_ID      = ${CMAKE_CXX_COMPILER_ID}"     )
message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_COMPILER_IS_GNUCXX   = ${CMAKE_COMPILER_IS_GNUCXX}"  )

#
# C++20
#

set (CMAKE_CXX_STANDARD 20)

#
# Build flags
#

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Ofast")

#
# Add custom modules.
#

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find_clang" ${CMAKE_MODULE_PATH})
include(cmake/pxx.cmake)

#
# Libraries.
#

find_package(Python COMPONENTS Interpreter Development)

#
# Find required packages
#

find_package(Eigen3)

# Python module binary
add_subdirectory(src)

# Python module
add_subdirectory(python)

# tests
add_subdirectory(test)

file(GLOB_RECURSE pxx_headers include "include/*.h")
file(GLOB_RECURSE pxx_templates include "include/*.template")
set(pxx_source_files "")
list(APPEND pxx_source_files ${pxx_headers})
list(APPEND pxx_source_files ${pxx_templates})
set(pxx_source_files ${pxx_source_files} CACHE INTERNAL "The PXX source files" FORCE)
