#include "catch2/catch.hpp"
#include "pxx/cxx/parser.h"
#include <filesystem>
#include <iostream>

TEST_CASE( "parse_namespace", "[cxx/parser]" ) {

    auto path = std::filesystem::path(__FILE__).parent_path();
    auto parser = pxx::cxx::Parser(path / "test_files" / "namespaces.h");

    pxx::cxx::Scope *scope;
    pxx::cxx::ASTNode *root;
    std::tie(root, scope) = parser.parse();

    // Qualified lookup
    auto symbol = scope->lookup_symbol("ns1");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);
    symbol = scope->lookup_symbol("ns1::ns2");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);
    symbol = scope->lookup_symbol("ns1::ns2::ns3");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);
    symbol = scope->lookup_symbol("ns1::ns2::ns4");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);

    // Unqualified lookup
    auto current_scope = symbol->get_scope();
    symbol = current_scope->lookup_symbol("ns1");
    REQUIRE(symbol->get_name() == "ns1");
    REQUIRE(symbol->get_qualified_name() == "ns1");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);

    symbol = current_scope->lookup_symbol("ns2");
    REQUIRE(symbol->get_name() == "ns2");
    REQUIRE(symbol->get_qualified_name() == "ns1::ns2");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);

    symbol = current_scope->lookup_symbol("ns3");
    REQUIRE(symbol->get_name() == "ns3");
    REQUIRE(symbol->get_qualified_name() == "ns1::ns2::ns3");
    REQUIRE(symbol->get_type() == pxx::cxx::ASTNodeType::NAMESPACE);

    root->print_tree(std::cout);

}
