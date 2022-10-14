add_subdirectory(cxx)
add_subdirectory(directives)

#add_executable(
#  test_main
#  main.cpp
#  cxx/test_string_manipulation.cpp
#  cxx/test_scope.cpp
#  cxx/test_type_parser.cpp
#  cxx/test_type_names.cpp
#  test_writer.cpp
#  )

add_executable(
  test_writer
  test_writer.cpp
)
target_link_libraries(test_writer PRIVATE Catch2::Catch2WithMain ${LIBCLANG_LIBRARIES})
target_include_directories(test_writer PUBLIC ${CMAKE_SOURCE_DIR}/include)

#find_package(LibClang)
#target_include_directories(test_main PUBLIC ${CMAKE_SOURCE_DIR}/include)
#target_link_libraries(test_main Catch2::Catch2 ${LIBCLANG_LIBRARIES})

#include(CTest)
#add_test(NAME unit_tests COMMAND test_main)


