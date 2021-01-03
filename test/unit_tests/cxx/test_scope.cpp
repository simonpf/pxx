#include <tuple>

#include "catch2/catch.hpp"
#include "pxx/cxx/string_manipulation.h"
#include "pxx/cxx/scope.h"

TEST_CASE( "Definition of types.", "[cxx/scope]" ) {

    using pxx::cxx::Scope;
    using pxx::cxx::detail::replace_names;

    Scope root_scope("root", nullptr);
    Scope my_namespace("my_namespace", &root_scope);
    Scope my_class_1("MyClass1", &my_namespace);
    Scope my_class_2("MyClass2", &my_namespace);

    REQUIRE( my_namespace.get_prefix() == "my_namespace::");
    REQUIRE( my_class_1.get_prefix() == "my_namespace::MyClass1::");

    //
    // Add some overloads.
    //

    my_class_1.add_name("foo");
    my_class_1.add_name("foo");
    REQUIRE(my_class_1.get_n_definitions("foo") == 2);

    //
    // Join namespaces.
    //

    my_class_2.add_name("foo");
    my_class_1.join(my_class_2);
    REQUIRE(my_class_1.get_n_definitions("foo") == 3);

    //
    // Add type alias.
    //

    my_namespace.add_type_alias("my_int", "int");
    my_class_1.add_type_alias("my_scalar", "float");

    auto replacements = my_class_1.get_type_replacements();
    auto result = replace_names("MyClass<my_int, my_scalar>",
                                std::get<0>(replacements),
                                std::get<1>(replacements));
    REQUIRE( result == "MyClass<int, float>" );

}

TEST_CASE( "Handling of children", "[cxx/scope]" ) {

    using pxx::cxx::Scope;
    using pxx::cxx::detail::replace_names;

    Scope root_scope("root", nullptr);
    Scope my_namespace("my_namespace", &root_scope);

    auto result = root_scope.get_child("my_namespace");
    REQUIRE(result == &my_namespace);

    Scope other_root("root", nullptr);
    Scope my_other_namespace("my_other_namespace", &other_root);
    root_scope.join(other_root);

    result = root_scope.get_child("my_other_namespace");
    REQUIRE(result == &my_other_namespace);
}

TEST_CASE( "Nested namespaces", "[cxx/scope]" ) {

    using pxx::cxx::Scope;
    using pxx::cxx::detail::replace_names;

    Scope root_scope("root", nullptr);
    Scope my_namespace("my_namespace", &root_scope);
    Scope nested("nested", &my_namespace);

    my_namespace.add_type_alias("nested", my_namespace.get_prefix() + nested.get_name());

    auto replacements = my_namespace.get_type_replacements();
    auto result = replace_names("nested::MyClass",
                                std::get<0>(replacements),
                                std::get<1>(replacements));
    REQUIRE( result == "my_namespace::nested::MyClass" );

}

TEST_CASE( "Name lookup", "[cxx/scope]" ) {

    using pxx::cxx::Scope;
    using pxx::cxx::detail::replace_names;

    Scope root_scope("root", nullptr);
    Scope my_namespace("my_namespace", &root_scope);
    Scope nested("nested", &my_namespace);

    nested.add_type_alias("nested", "int");

    auto result = root_scope.lookup_name("my_namespace::nested::nested");
    REQUIRE(result == "int");

    nested.add_name("MyClass");
    result = my_namespace.lookup_name("nested::MyClass");
    REQUIRE(result == "my_namespace::nested::MyClass");

    my_namespace.set_name("other");
    result = my_namespace.lookup_name("nested::MyClass");
    REQUIRE(result == "other::nested::MyClass");

    result = my_namespace.lookup_name("int");
    REQUIRE(result == "int");

}

TEST_CASE( "Name replacement", "[cxx/scope]" ) {

    using pxx::cxx::Scope;
    using pxx::cxx::detail::replace_names;

    Scope root_scope("root", nullptr);
    Scope my_namespace("my_namespace", &root_scope);
    Scope nested("nested", &my_namespace);

    nested.add_type_alias("nested", "int");


    nested.add_name("MyClass");
    auto result = nested.get_qualified_name("nested");
    REQUIRE(result == "int");

    result = nested.get_qualified_name("::nested");
    REQUIRE(result == "::nested");

    result = nested.get_qualified_name("MyClass<nested>");
    REQUIRE(result == "my_namespace::nested::MyClass<int>");

}
