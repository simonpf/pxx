macro(add_pxx_module)

  ##############################################################################
  # Command line arguments
  ##############################################################################

  set(options "")
  set(oneValueArgs SOURCE MODULE)
  set(multiValueArgs INCLUDE_DIRS)
  cmake_parse_arguments(PXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  set(output_file ${CMAKE_CURRENT_BINARY_DIR}/${PXX_MODULE}.cxx)
  set(input_file ${CMAKE_CURRENT_SOURCE_DIR}/${PXX_SOURCE})

  if (PXX_INCLUDE_DIRS)
    string(JOIN ":" include_arg ${PXX_INCLUDE_DIRS})
    set(include_arg "-I${include_arg}")
  else()
    set(include_arg)
  endif()

  ##############################################################################
  # Comman line arguments
  ##############################################################################

  find_package(Python COMPONENTS Interpreter Development)

  add_custom_command(
    OUTPUT ${output_file}
    COMMAND ${Python_EXECUTABLE} -m pxx ${input_file} --output_file ${output_file} ${include_arg}
    DEPENDS ${pxx_source_files} _pxx ${input_file} pxx_python
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/python)

  add_library(${PXX_MODULE} SHARED ${output_file})

  execute_process(
    COMMAND ${Python_EXECUTABLE} -c "import pxx, os; print(os.path.dirname(pxx.__file__))"
    OUTPUT_VARIABLE pxx_include_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  get_filename_component(input_folder ${input_file} DIRECTORY)

  target_include_directories(
    ${PXX_MODULE}
    PUBLIC ${Python_INCLUDE_DIRS} "${pxx_include_path}/include/" ${input_folder} ${CMAKE_CURRENT_SOURCE_DIR}
    )

  set_target_properties(${PXX_MODULE} PROPERTIES PREFIX "")
endmacro()
