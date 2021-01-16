#include "catch2/catch.hpp"
#include "pxx/cxx/type_expression.h"

TEST_CASE( "identifier_iterator", "[cxx/type_name]" ) {

    using pxx::cxx::types::IdentifierIterator;
    using pxx::cxx::types::IdentifierType;

    std::string test_type = "const std::my_namespace::Template<int, MyType>";

    auto iterator = IdentifierIterator(test_type);

    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "std");
    REQUIRE(iterator.get_type() == IdentifierType::qualifier);

    ++iterator;
    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "my_namespace");
    REQUIRE(iterator.get_type() == IdentifierType::qualifier);

    ++iterator;
    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "Template");
    REQUIRE(iterator.get_type() == IdentifierType::template_name);

    ++iterator;
    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "MyType");
    REQUIRE(iterator.get_type() == IdentifierType::type_name);

    ++iterator;
    REQUIRE(iterator.get_type() == IdentifierType::end);
    REQUIRE(!iterator);

    test_type = "void (*)(int, MyClass, &MyOtherClass)";
    iterator = IdentifierIterator(test_type);
    REQUIRE(iterator.get_identifier() == "MyClass");
    REQUIRE(iterator.get_type() == IdentifierType::type_name);
    ++iterator;
    REQUIRE(iterator.get_identifier() == "MyOtherClass");
    REQUIRE(iterator.get_type() == IdentifierType::type_name);

}

TEST_CASE( "identifier_iterator_replacements", "[cxx/type_name]" ) {

    using pxx::cxx::types::IdentifierIterator;
    using pxx::cxx::types::IdentifierType;

    std::string test_type = "const std::my_namespace::Template<int, MyType>";

    auto iterator = IdentifierIterator(test_type);

    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "std");
    REQUIRE(iterator.get_type() == IdentifierType::qualifier);
    iterator = "my_namespace";
    REQUIRE(iterator.get_identifier() == "my_namespace");

    ++iterator;
    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "my_namespace");
    REQUIRE(iterator.get_type() == IdentifierType::qualifier);
    iterator = "std";
    REQUIRE(iterator.get_identifier() == "std");

    ++iterator;
    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "Template");
    REQUIRE(iterator.get_type() == IdentifierType::template_name);

    ++iterator;
    REQUIRE(iterator);
    REQUIRE(iterator.get_identifier() == "MyType");
    REQUIRE(iterator.get_type() == IdentifierType::type_name);
    iterator = "MyOtherType";

    REQUIRE(test_type == "const my_namespace::std::Template<int, MyOtherType>");


}
