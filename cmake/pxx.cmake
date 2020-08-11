macro(add_pxx_module)

  ##############################################################################
  # Command line arguments
  ##############################################################################

  set(options "")
  set(oneValueArgs SOURCE MODULE)
  set(multiValueArgs INCLUDE_DIRS DEPENDS)
  cmake_parse_arguments(PXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  set(output_file ${CMAKE_CURRENT_BINARY_DIR}/${PXX_MODULE}.cxx)
  set(input_file ${PXX_SOURCE})
  if (NOT IS_ABSOLUTE ${input_file})
    get_filename_component(input_file ${input_file} ABSOLUTE)
  endif()

  set(include_args ${PXX_INCLUDE_DIRS})
  list(TRANSFORM include_args PREPEND -I)

  ##############################################################################
  # Command line arguments
  ##############################################################################

  find_package(Python COMPONENTS Interpreter Development)
  add_library(${PXX_MODULE} SHARED ${output_file})

  #
  # Use right CXX standard
  #

  get_target_property(CS ${PXX_MODULE} CXX_STANDARD)
  set(STANDARDFLAG "-std=c++${CS}")

  add_custom_command(
    OUTPUT ${output_file}
    COMMAND ${Python_EXECUTABLE} -m pxx ${input_file} --output_file ${output_file} ${STANDARDFLAG} ${include_args}
    DEPENDS ${input_file})


  execute_process(
    COMMAND ${Python_EXECUTABLE} -c "import pxx, os; print(os.path.dirname(pxx.__file__))"
    OUTPUT_VARIABLE pxx_include_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  get_filename_component(input_folder ${input_file} DIRECTORY)

  target_include_directories(
    ${PXX_MODULE}
    PUBLIC
    ${Python_INCLUDE_DIRS}
    "${pxx_include_path}/include/"
    ${input_folder}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PXX_INCLUDE_DIRS}
    )

  set_target_properties(${PXX_MODULE} PROPERTIES PREFIX "")
endmacro()
