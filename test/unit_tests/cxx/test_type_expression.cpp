#include "catch2/catch.hpp"
#include "pxx/cxx/type_expression.h"

TEST_CASE( "identifier_iterator", "[cxx/type_name]" ) {

    using pxx::cxx::types::IdentifierIterator;
    using pxx::cxx::types::IdentifierType;

    std::string test_type = "const my_namespace::Template<int, MyType>";

    auto iterator = IdentifierIterator(test_type);

    REQUIRE(iterator.get_identifier() == "my_namespace");
    REQUIRE(iterator.get_type() == IdentifierType::qualifier);

    ++iterator;
    REQUIRE(iterator.get_identifier() == "Template");
    REQUIRE(iterator.get_type() == IdentifierType::template_name);

    ++iterator;
    REQUIRE(iterator.get_identifier() == "MyType");
    REQUIRE(iterator.get_type() == IdentifierType::type_name);




}
