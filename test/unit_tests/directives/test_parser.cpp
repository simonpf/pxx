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

    auto input = Input("=identifier,  another_identifier \"a string\"");

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
    REQUIRE(token.content == "a string");

    token = Token{input};
    REQUIRE(token.type == TokenType::END);

}
