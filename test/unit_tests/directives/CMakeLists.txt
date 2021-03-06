add_executable(
  test_directives_parser
  test_parser.cpp
  )
target_include_directories(test_directives_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_directives_parser Catch2::Catch2 ${LIBCLANG_LIBRARIES})
