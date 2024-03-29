
#
# Version from git
#

execute_process(
  COMMAND git describe --tags --abbrev=0
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION
  )
string(REGEX REPLACE "\n$" "" VERSION "${VERSION}")
set(PXX_BINARY_DIR ${CMAKE_BINARY_DIR})
set(PXX_SOURCE_DIR ${CMAKE_SOURCE_DIR})

configure_file(setup.py setup.py @ONLY)
configure_file(pxx/__init__.py pxx/__init__.py @ONLY)
configure_file(pxx/__main__.py pxx/__main__.py @ONLY)

add_custom_target(pxx_python
  ALL
  COMMAND ${Python3_EXECUTABLE} setup.py build
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS pxx/__init__.py pxx/__main__.py setup.py _pxx
  COMMENT "Building pxx Python package.")

#
# Copy clang headers
#

find_package(LibClang)
file(GLOB_RECURSE clang_headers "${LIBCLANG_BUILTIN_DIR}/*h")

foreach(file ${clang_headers})
  file(RELATIVE_PATH output_file ${LIBCLANG_BUILTIN_DIR} ${file})
  configure_file(${file} pxx/include/clang/${output_file})
endforeach(file ${clang_headers})

#
# Copy clang headers
#

find_package(LibClang)
set(pybind_dir "${CMAKE_SOURCE_DIR}/ext/pybind11")
file(GLOB_RECURSE pybind_headers "${pybind_dir}/*h")

foreach(file ${pybind_headers})
  file(RELATIVE_PATH output_file "${CMAKE_SOURCE_DIR}/ext" ${file})
  configure_file(${file} pxx/include/${output_file})
endforeach(file ${pybind_headers})

#
# Copy pxx.cmake
#

configure_file(${CMAKE_SOURCE_DIR}/cmake/pxx.cmake pxx/cmake/pxx.cmake)
