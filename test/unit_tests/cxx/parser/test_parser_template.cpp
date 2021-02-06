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


}
