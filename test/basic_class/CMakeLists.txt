configure_file(test_class.py test_class.py)

include_directories("${PROJECT_SOURCE_DIR}/ext/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories(${Python_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(input_file ${CMAKE_CURRENT_SOURCE_DIR}/basic_class.h)
set(output_file ${CMAKE_CURRENT_BINARY_DIR}/basic_class.cxx)
message(WARNING input_file: ${input_file})
message(WARNING output_file: ${output_file})
add_custom_command(
  OUTPUT ${output_file}
  COMMAND ${Python_EXECUTABLE} -m pxx ${input_file} --output_file ${output_file}
  WORKGIN_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python)

add_library(basic_class SHARED ${output_file})
set_target_properties(basic_class PROPERTIES PREFIX "")
