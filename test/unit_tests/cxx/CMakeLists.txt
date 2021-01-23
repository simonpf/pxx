add_executable(
  test_parser
  test_parser.cpp
  parser/test_parser_class.cpp
  parser/test_parser_namespace.cpp
  )

find_package(LibClang)
target_include_directories(test_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_parser Catch2::Catch2 ${LIBCLANG_LIBRARIES})

include(CTest)
add_test(NAME cxx_parser COMMAND test_parser)

