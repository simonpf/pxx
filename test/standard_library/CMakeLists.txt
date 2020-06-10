configure_file(test_vector.py test_vector.py)

include_directories("${PROJECT_SOURCE_DIR}/ext/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories(${Python_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(input_file ${CMAKE_CURRENT_SOURCE_DIR}/vector.h)
set(output_file ${CMAKE_CURRENT_BINARY_DIR}/vector.cxx)
set(pxx_source_files CACHE INTERNAL "The PXX source files")
message(WARNING input_file: ${input_file})
message(WARNING output_file: ${output_file})
add_custom_command(
  OUTPUT ${output_file}
  COMMAND ${Python_EXECUTABLE} -m pxx ${input_file} --output_file ${output_file}
  DEPENDS ${pxx_source_files} _pxx ${input_file} pxx_python
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python)

add_library(vector SHARED ${output_file})
set_target_properties(vector PROPERTIES PREFIX "")
