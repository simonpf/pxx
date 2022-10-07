#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"


#include "pxx/cxx/scope.h"
#include "pxx/cxx/ast.h"
#include "pxx/cxx/parser.h"
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

TEST_CASE( "replace_type_names", "[cxx/type_name]" ) {

    using pxx::cxx::Parser;
    using pxx::cxx::Scope;
    using pxx::cxx::ASTNode;
    using pxx::cxx::types::replace_type_names;

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = Parser(path / "test_files" / "namespaces.h");
    Scope *scope;
    ASTNode *root;
    std::tie(root, scope) = parser.parse();

    auto child = scope->get_child_scope("a");

    std::string result = replace_type_names("c::A", child);
    REQUIRE(result == "a::c::A");
    result = replace_type_names("std::vector<c::A>", child);
    REQUIRE(result == "std::vector<a::c::A>");

    child = child->get_child_scope("c");
    result = replace_type_names("A", child);
    REQUIRE(result == "a::c::A");
    result = replace_type_names("std::vector<A>", child);
    REQUIRE(result == "std::vector<a::c::A>");

    child = scope->get_child_scope("a")->get_child_scope("b");
    result = replace_type_names("c::A", child);
    REQUIRE(result == "a::c::A");
    result = replace_type_names("std::vector<c::A>", child);
    REQUIRE(result == "std::vector<a::c::A>");

    result = replace_type_names("a::c::A", scope);
    REQUIRE(result == "a::c::A");
    result = replace_type_names("std::vector<a::c::A>", child);
    REQUIRE(result == "std::vector<a::c::A>");
}
