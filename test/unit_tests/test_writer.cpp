#include <iostream>
#include <sstream>

#include "catch2/catch_test_macros.hpp"
#include "pxx/writer.h"
#include "pxx/cxx/parser.h"


TEST_CASE( "file_header", "[cxx/writer]" ) {

    using pxx::Writer;
    using pxx::cxx::Scope;
    using pxx::cxx::ASTNode;

    std::stringstream output{};
    auto writer = Writer{output};

    Scope scope{};
    scope.add_child_scope("std");
    scope.add_child_scope("Eigen");

    writer.write(&scope, nullptr, {});

    std::cout << output.str() << std::endl;

}

TEST_CASE( "write_class", "[cxx/writer]" ) {

    using pxx::Writer;
    using pxx::cxx::Parser;
    using pxx::cxx::Scope;
    using pxx::cxx::ASTNode;

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = Parser(path / "cxx" / "parser" / "test_files" / "simple_class.h");
    Scope* scope;
    ASTNode* ast;
    std::tie(ast, scope) = parser.parse();

    std::stringstream output{};
    auto writer = Writer{output};
    writer.write(scope, ast, {});

    std::cout << output.str() << std::endl;
}
