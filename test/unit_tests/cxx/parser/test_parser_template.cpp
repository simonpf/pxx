#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

using namespace pxx::cxx;

TEST_CASE( "parse_template", "[cxx/parser]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = Parser(path / "test_files" / "templates.h");

    Scope *scope;
    ASTNode *root;
    std::tie(root, scope) = parser.parse();
    auto children = root->get_children();

    auto function = reinterpret_cast<Overload<Function>*>(children["function"]);
    REQUIRE(function->get_type() == ASTNodeType::FUNCTION_TEMPLATE);

    auto cl = dynamic_cast<ClassTemplate*>(children["Class"]);
    REQUIRE(cl->get_type() == ASTNodeType::CLASS_TEMPLATE);


    auto& instances = cl->get_instances();
    REQUIRE(instances.size() == 1);

    auto& specializations = cl->get_specializations();
    REQUIRE(specializations.size() == 1);

    cl = dynamic_cast<ClassTemplate*>(scope->lookup_symbol("test::OtherClass"));
    REQUIRE(cl->get_type() == ASTNodeType::CLASS_TEMPLATE);
    auto& other_instances = cl->get_instances();
    REQUIRE(other_instances.size() == 1);
    auto& other_specializations = cl->get_specializations();
    REQUIRE(other_specializations.size() == 1);

}
