#define CATCH_CONFIG_MAIN

#include <sstream>

#include "catch2/catch.hpp"

#include "pxx/directives/parser.h"


TEST_CASE( "Input stream", "[directives/input]" ) {

    using pxx::directives::Input;

    auto input = Input("   test .");

    std::stringstream stream;
    for (size_t i = 0; i < 8; ++i) {
        char c = input.consume();
        if (c) {
          stream << c;
        }
    }

    REQUIRE(stream.str() == "test.");

}

TEST_CASE( "Token extraction", "[directives/token]" ) {

    using pxx::directives::Input;
    using pxx::directives::Token;
    using pxx::directives::TokenType;

    auto input = Input("=identifier,  another_identifier \"  a  string   \" ");

    Token token{input};
    REQUIRE(token.type == TokenType::EQUAL);
    REQUIRE(token.content == "=");

    token = Token{input};
    REQUIRE(token.type == TokenType::IDENTIFIER);
    REQUIRE(token.content == "identifier");

    token = Token{input};
    REQUIRE(token.type == TokenType::COMMA);
    REQUIRE(token.content == ",");

    token = Token{input};
    REQUIRE(token.type == TokenType::IDENTIFIER);
    REQUIRE(token.content == "another_identifier");

    token = Token{input};
    REQUIRE(token.type == TokenType::STRING);
    REQUIRE(token.content == "  a  string   ");

    token = Token{input};
    REQUIRE(token.type == TokenType::END);

}

TEST_CASE( "String extraction", "[directives/token]" ) {

    using pxx::directives::Input;
    using pxx::directives::Token;
    using pxx::directives::TokenType;

    auto input = Input("\n \"some string \" \" a string with \\\"  \"");

    Token token{input};
    REQUIRE(token.type == TokenType::STRING);
    REQUIRE(token.content == "some string ");

    token = Token{input};
    REQUIRE(token.type == TokenType::STRING);
    REQUIRE(token.content == " a string with \\\"  ");
}

TEST_CASE( "Directive parsing", "[directives/parser]" ) {

    using pxx::directives::Parser;

    std::string test_comment =
        R"(
adsfads
 /// a comment to ignore.
// pxx :: name = "test", export = false
other stuff
// pxx :: export = true
         )";

    auto parser = Parser(test_comment);
    auto export_settings = parser.parse();

    REQUIRE(export_settings[0].name == "test");
    REQUIRE(export_settings[0].exported == false);

    REQUIRE(export_settings[1].exported);

}
