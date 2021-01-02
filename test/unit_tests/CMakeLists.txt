add_executable(
  test_main
  main.cpp
  cxx/test_string_manipulation.cpp
  cxx/test_scope.cpp
  cxx/test_type_parser.cpp
  )
target_include_directories(test_main PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_main Catch2::Catch2)

include(CTest)
add_test(NAME unit_tests COMMAND test_main)


