#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

TEST_CASE( "parse_function", "[cxx/parser]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = pxx::cxx::Parser(path / "test_files" / "functions.h");

    pxx::cxx::Scope *scope;
    pxx::cxx::ASTNode *root;
    std::tie(root, scope) = parser.parse();

    auto children = root->get_children();
    auto function = reinterpret_cast<pxx::cxx::FunctionOverload *>(children["function1"]);

    REQUIRE(function->get_type() == pxx::cxx::ASTNodeType::FUNCTION);
    REQUIRE(function->get_name() == "function1");
    REQUIRE(function->get_n_overloads() == 2);

    function = reinterpret_cast<pxx::cxx::FunctionOverload *>(
        scope->lookup_symbol("my_namespace::function2")
        );

    REQUIRE(function->get_type() == pxx::cxx::ASTNodeType::FUNCTION);
    REQUIRE(function->get_name() == "function2");
    REQUIRE(function->get_qualified_name() == "my_namespace::function2");
    REQUIRE(function->get_n_overloads() == 1);

    std::cout << *root << std::endl;
}
