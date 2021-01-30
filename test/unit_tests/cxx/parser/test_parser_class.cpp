#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

TEST_CASE( "parse_class", "[cxx/scope]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = pxx::cxx::Parser(path / "test_files" / "simple_class.h");

    pxx::cxx::Scope *scope;
    pxx::cxx::ASTNode *root;
    std::tie(root, scope) = parser.parse();

    auto children = root->get_children();

    auto member = reinterpret_cast<pxx::cxx::ClassMethodOverload *>(children["A"]);

    std::cout << *member << std::endl;

}
