#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

TEST_CASE( "parse_class", "[cxx/scope]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();

    auto parser = pxx::cxx::Parser(path / "test_files" / "simple_class.h");
    auto parsed = parser.parse();

    std::get<0>(parsed)->print_tree(std::cout);


}
