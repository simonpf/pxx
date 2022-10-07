#include "catch2/catch_test_macros.hpp"

#include "pxx/cxx/string_manipulation.h"

TEST_CASE( "Removal of template arguments.", "[cxx/string_manipulation]" ) {

    using pxx::cxx::detail::remove_template_arguments;

    auto result = remove_template_arguments("MyClass<int, char>");
    REQUIRE( result == "MyClass");

    result = remove_template_arguments("my_namespace::MyClass<int, char>");
    REQUIRE( result == "my_namespace::MyClass");

    result = remove_template_arguments("MyOtherClass<std::string>::MyClass<int, char>");
    REQUIRE( result == "MyOtherClass<std::string>::MyClass");

}

TEST_CASE( "Replacement of type names.", "[cxx/string_manipulation]" ) {

    using pxx::cxx::detail::replace_names;

    auto result = replace_names("MyClass<Scalar, N>",
                                {"Scalar", "N"},
                                {"float", "3"});
    REQUIRE(result == "MyClass<float, 3>");

    result = replace_names("my_namespace::MyClass<Scalar, N>",
                           {"Scalar", "N"},
                           {"float", "3"});
    REQUIRE( result == "my_namespace::MyClass<float, 3>");

    result = replace_names("my_namespace::MyClass1<Scalar>::MyClass2<N>",
                           {"Scalar", "N"},
                           {"float", "3"});
    REQUIRE( result == "my_namespace::MyClass1<float>::MyClass2<3>");
}

TEST_CASE( "Replacement of unqualified type names.", "[cxx/string_manipulation]" ) {

    using pxx::cxx::detail::replace_unqualified_names;

    auto result = replace_unqualified_names("MyClass<Scalar, N>",
                                            {"Class", "Scalar", "N"},
                                            {"Error", "float", "3"});
    REQUIRE(result == "MyClass<float, 3>");

    result = replace_unqualified_names("my_namespace::MyClass<Scalar, N>",
                           {"Scalar", "N"},
                           {"float", "3"});
    REQUIRE( result == "my_namespace::MyClass<float, 3>");

    result = replace_unqualified_names("my_namespace::MyClass1<Scalar>::MyClass2<N>",
                           {"Scalar", "N"},
                           {"float", "3"});
    REQUIRE( result == "my_namespace::MyClass1<float>::MyClass2<3>");
}
