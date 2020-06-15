
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
  COMMAND ${Python_EXECUTABLE} setup.py build
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS pxx pxx/__init__.py pxx/__main__.py
  COMMENT "Building pxx Python package.")

find_package(LibClang)
string(REGEX REPLACE "\n$" "" VERSION "${VERSION}")
file(GLOB_RECURSE clang_headers "${LIBCLANG_BUILTIN_DIR}/*h")

foreach(file ${clang_headers})
  file(RELATIVE_PATH output_file ${LIBCLANG_BUILTIN_DIR} ${file})
  configure_file(${file} pxx/include/${output_file})
endforeach(file IN ${clang_headers})

