#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

using namespace pxx::cxx;

TEST_CASE( "parse_function", "[cxx/parser]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = Parser(path / "test_files" / "functions.h");

    Scope *scope;
    ASTNode *root;
    std::tie(root, scope) = parser.parse();

    auto children = root->get_children();
    auto function = reinterpret_cast<Overload<Function>*>(children["function1"]);

    REQUIRE(function->get_type() == ASTNodeType::FUNCTION);
    REQUIRE(function->get_name() == "function1");
    REQUIRE(function->get_n_overloads() == 2);

    function = reinterpret_cast<Overload<Function>*>(
        scope->lookup_symbol("my_namespace::function2")
        );

    REQUIRE(function->get_type() == ASTNodeType::FUNCTION);
    REQUIRE(function->get_name() == "function2");
    REQUIRE(function->get_qualified_name() == "my_namespace::function2");
    REQUIRE(function->get_n_overloads() == 1);

    std::cout << *root << std::endl;
}
