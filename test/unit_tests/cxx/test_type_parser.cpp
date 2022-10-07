#include <tuple>
#include <sstream>

#include "catch2/catch_test_macros.hpp"

#include "pxx/cxx/type_parser.h"

TEST_CASE( "Printing of basic types", "[cxx/type_parser]" ) {

    std::stringstream out;
    pxx::types::Type t_0("int");
    pxx::types::Type t_1("my_namespace", "MyInt", {});
    pxx::types::Type t_2("int", {"const", "*"});

    //
    // Basic type
    //

    out << t_0;
    REQUIRE( out.str() == "int");

    out = std::stringstream();
    out << t_1;
    REQUIRE( out.str() == "my_namespace::MyInt");

    out = std::stringstream();
    out << t_2;
    REQUIRE( out.str() == "int const *");

    //
    // Type template
    //

    std::vector<pxx::types::TypePtr> arguments = {};
    arguments.push_back(std::make_unique<pxx::types::Type>(t_0));
    arguments.push_back(std::make_unique<pxx::types::Type>(t_1));
    arguments.push_back(std::make_unique<pxx::types::Type>(t_2));

    pxx::types::TypeTemplate my_template("my_scope", "my_template", {"&"}, arguments);
    out = std::stringstream();
    out << my_template;
    REQUIRE( out.str() == "my_scope::my_template<int, my_namespace::MyInt, int const *> &");

    //
    // Function pointer type
    //

    arguments.resize(0);
    arguments.push_back(std::make_unique<pxx::types::Type>(t_0));
    arguments.push_back(std::make_unique<pxx::types::Type>(t_1));
    arguments.push_back(std::make_unique<pxx::types::Type>(t_2));
    pxx::types::TypePtr t_0_ptr(new pxx::types::Type(t_0));
    pxx::types::FunctionPointerType function_ptr(t_0_ptr,
                                                 arguments);
    out = std::stringstream();
    out << function_ptr;
    REQUIRE( out.str() == "int (*)(int, my_namespace::MyInt, int const *)");


    //
    // Member pointer type
    //

    pxx::types::TypePtr t_ptr(new pxx::types::Type("int"));
    pxx::types::TypePtr p_ptr(new pxx::types::Type("C"));
    pxx::types::MemberPointerType member_ptr(t_ptr, p_ptr);
    out = std::stringstream();
    out << member_ptr;
    REQUIRE( out.str() == "int C::*");

}

TEST_CASE( "Source", "[cxx/type_parser]" ) {

    pxx::types::Source source("int &");

    REQUIRE(source.next_char() == 'i');
    REQUIRE(source.current_char() == 'i');
    REQUIRE(source.peek_char() == 'n');
    REQUIRE(source.peek<3>() == std::array<char, 3>{'i', 'n', 't'});

    REQUIRE(source.next_char() == 'n');
    REQUIRE(source.current_char() == 'n');
    REQUIRE(source.peek_char() == 't');
    REQUIRE(source.peek<5>() == std::array<char, 5>{'n', 't', ' ', '&', pxx::types::Source::EOL});

    REQUIRE(source.next_char() == 't');
    REQUIRE(source.next_char() == ' ');
    REQUIRE(source.next_char() == '&');
    REQUIRE(source.next_char() == pxx::types::Source::EOL);
    REQUIRE(source.next_char() == pxx::types::Source::EOL);
}
