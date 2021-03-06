#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

using namespace pxx::cxx;

TEST_CASE( "parse_class", "[cxx/parser]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = Parser(path / "test_files" / "simple_class.h");
    Scope *scope;
    ASTNode *root;
    std::tie(root, scope) = parser.parse();

    // Constructors
    auto constructors = reinterpret_cast<Overload<Constructor>*>(scope->lookup_symbol("A::A"));
    REQUIRE(constructors->get_n_overloads() == 2);

    // Public member methods and variables.
    auto public_member = reinterpret_cast<MemberVariable*>(
        scope->lookup_symbol("A::public_member")
        );
    REQUIRE(public_member->get_accessibility() == Accessibility::PUBLIC);
    REQUIRE(public_member->get_qualified_name() == "A::public_member");
    auto public_method = reinterpret_cast<Overload<MemberFunction>*>(
        scope->lookup_symbol("A::public_method_1")
        );
    REQUIRE(public_method->get_accessibility() == Accessibility::PUBLIC);
    REQUIRE(public_method->get_n_overloads() == 2);
    REQUIRE(public_method->get_qualified_name() == "A::public_method_1");
    public_method = reinterpret_cast<Overload<MemberFunction>*>(
        scope->lookup_symbol("A::public_method_2")
        );
    REQUIRE(public_method->get_accessibility() == Accessibility::PUBLIC);
    REQUIRE(public_method->get_n_overloads() == 1);

    // Private member methods and variables.
    auto private_member = reinterpret_cast<MemberVariable*>(
        scope->lookup_symbol("A::private_member")
        );
    REQUIRE(private_member->get_accessibility() == Accessibility::PRIVATE);
    auto private_method = reinterpret_cast<Overload<MemberFunction>*>(
        scope->lookup_symbol("A::private_method")
        );
    REQUIRE(private_method->get_accessibility() == Accessibility::PRIVATE);

    // Protected member methods and variables.
    auto protected_member = reinterpret_cast<MemberVariable*>(
        scope->lookup_symbol("A::protected_member")
        );
    REQUIRE(protected_member->get_accessibility() == Accessibility::PROTECTED);
    auto protected_method = reinterpret_cast<Overload<MemberFunction>*>(
        scope->lookup_symbol("A::protected_method")
        );
    REQUIRE(protected_method->get_accessibility() == Accessibility::PROTECTED);
}
