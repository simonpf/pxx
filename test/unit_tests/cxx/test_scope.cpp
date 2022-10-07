#include <tuple>

#include "catch2/catch_test_macros.hpp"
#include "pxx/cxx/scope.h"
#include "pxx/cxx/ast.h"

TEST_CASE( "Definition of types.", "[cxx/scope]" ) {

    using pxx::cxx::Scope;

    Scope root_scope{};
    auto namespace_1 = root_scope.add_child_scope("namespace");
    auto class_1 = namespace_1->add_child_scope("Class1");
    auto class_2 = namespace_1->add_child_scope("Class2");

    REQUIRE(namespace_1->get_prefix() == "namespace::");
    REQUIRE(class_1->get_prefix() == "namespace::Class1::");

    auto class_2_2 = namespace_1->get_child_scope("Class2");
    REQUIRE(class_2_2 == class_2);
    class_2_2 = root_scope.get_child_scope("namespace::Class2");
    REQUIRE(class_2_2 == class_2);

}

TEST_CASE( "Handling of children", "[cxx/scope]" ) {

    using pxx::cxx::Scope;

    Scope root_scope{};
    auto namespace_1 = root_scope.add_child_scope("namespace");
    auto result = root_scope.get_child_scope("namespace");

    REQUIRE(result == namespace_1);
}

TEST_CASE( "std and Eigen headers.", "[cxx/scope]" ) {

    using pxx::cxx::Scope;

    Scope root_scope{};
    root_scope.add_child_scope("std");
    root_scope.add_child_scope("Eigen");

    REQUIRE(root_scope.has_std_namespace());
    REQUIRE(root_scope.has_eigen_namespace());
}

TEST_CASE( "Replacement of type names", "[cxx/scope]" ) {

    using pxx::cxx::Scope;

    Scope root_scope{};
    root_scope.add_child_scope("std");
    root_scope.add_child_scope("Eigen");

    REQUIRE(root_scope.has_std_namespace());
    REQUIRE(root_scope.has_eigen_namespace());
}
