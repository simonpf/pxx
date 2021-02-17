#include <iostream>
#include <sstream>

#include "catch2/catch.hpp"
#include "pxx/writer.h"


TEST_CASE( "file_header", "[cxx/writer]" ) {

    using pxx::Writer;
    using pxx::cxx::Scope;
    using pxx::cxx::ASTNode;

    std::stringstream output("test");
    auto writer = Writer{output};

    Scope scope{};
    scope.add_child_scope("std");
    scope.add_child_scope("Eigen");

    writer.write(&scope, nullptr, {});

    std::cout << output.str() << std::endl;

}
