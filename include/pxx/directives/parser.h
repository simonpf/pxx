/** \file pxx/directives/parser.h
 *
 * This file defines a parser to parse pxx directives.
 *
 */
#ifndef __PXX_DIRECTIVES_PARSER_H__
#define __PXX_DIRECTIVES_PARSER_H__

#include <regex>
#include <string>

namespace pxx {
namespace directives {

namespace {
std::regex pxx_directive{"[ \t]*//[ \t]*pxx[ \t]*::([^\n]*)",
                         std::regex::extended};

} // namespace directives

namespace detail {
    inline bool is_identifier_char(char c) { return (isalnum(c) || c == '_'); }
}

///////////////////////////////////////////////////////////////////////////////
// Input
///////////////////////////////////////////////////////////////////////////////
/** Input class to consume directive strings.
 *
 * This class provides an interface to iterate over the character symbols in
 * a directive while ignoring whitespace characters.
 */
class Input {

private:

  void drop_whitespace() {
    while (iswspace(current_char())) {
      ++pos_;
    }
  }

public:
  Input(const std::string &input) : input_(input), n_chars_(input.size()) {
    drop_whitespace();
  }

  /// Return next non-whitespace character.
  char peek() {
    size_t pos = pos_ + 1;
    while ((pos < n_chars_) && iswspace(input_[pos_ + 1])) {
      ++pos;
    }
    if (pos >= n_chars_) {
      return 0;
    }
    return input_[pos];
  }

  /** Get the current char without advancing to
   * the next one.
   */
  char current_char() {
    if (pos_ >= n_chars_) {
      return 0;
    }
    return input_[pos_];
  }

  /** Consume char from input.
   *
   * @return The char at the current position of the input stream
   * or 0 if input it is exhausted.
   */
  char consume() {
    drop_whitespace();
    char c = current_char();
    ++pos_;
    drop_whitespace();
    return c;
  }

  size_t get_position() const { return pos_; }

  std::string extract(size_t start, size_t length) {
    return std::string(input_, start, length);
  }

private:
  std::string input_;
  size_t pos_ = 0;
  size_t n_chars_ = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Tokens
///////////////////////////////////////////////////////////////////////////////

enum class TokenType { IDENTIFIER, COMMA, EQUAL, STRING, UNKNOWN, END };

struct Token {
public:
  Token() {}
  Token(Input &input) {
    start = input.get_position();
    char c = input.consume();

    if (c == 0) {
      type = TokenType::END;
      content = input.extract(start, length);
      return;
    }

    if (c == ',') {
      type = TokenType::COMMA;
      length = 1;
      content = input.extract(start, length);
      return;
    }

    if (c == '=') {
      type = TokenType::EQUAL;
      length = 1;
      content = input.extract(start, length);
      return;
    }

    if (c == '\"') {
      type = TokenType::STRING;
      char previous = 0;
      size_t end = start;
      do {
          end = input.get_position();
          previous = c;
          c = input.consume();
          ++length;
          if (c == 0) break;
      } while ((previous == '\\') || (c != '\"'));
      content = input.extract(start + 1, end - start - 1);
      return;
    }

    if (detail::is_identifier_char(c)) {
      type = TokenType::IDENTIFIER;
      while (detail::is_identifier_char(input.current_char())) {
        input.consume();
        ++length;
      }
      content = input.extract(start, length);
      return;
    }
  }

  TokenType type;
  size_t start = 0;
  size_t length = 1;
  std::string content;
};

////////////////////////////////////////////////////////////////////////////////
// TokenStream
////////////////////////////////////////////////////////////////////////////////
/** A stream of tokens.
 *
 * Helper class to iterate over a stream of tokens. Can be converted
 * to a bool value to check whether the stream is exhausted.
 */
struct TokenStream {

  /** Create TokenStream.
   *
   * Creates a TokenStream to iterate over tokens in a string.
   *
   * @param input_: The input string to iterate over.
   */
  TokenStream(const std::string input_) : input(input_) {
    token = Token(input);
  }

  /** Get next token from stream.
   *
   * Sets the token member of this class to the next token in the
   * stream.
   */
  TokenStream &operator++() {
    if (*this) {
      token = Token(input);
    }
    return *this;
  }

  /// Determine whether stream is exhausted.
  operator bool() { return token.type != TokenType::END; }

  Input input;
  Token token;
};

class Lexer {

  Lexer(std::string input) : input_(input) {}

  Token next() {
    Input input{input_.str()};
    return Token(input);
  }


private:

    std::stringstream input_;
};

///////////////////////////////////////////////////////////////////////////////
// ExportSettings
///////////////////////////////////////////////////////////////////////////////

/** ExportSettings
 *
 * Struct to hold the results from a parsed pxx directive.
 *
 */
struct ExportSettings {

  // Whether or not the AST node should be exported.
  bool exported;
  // The name to used when exporting the node.
  std::string name;
  // String of template parameters when node is a template.
  std::vector<std::string> parameters;

  /** Extract export settings from token stream.
   *
   * @param stream A token stream from a pxx export directive to be parsed.
   */
  ExportSettings(TokenStream &stream) {

    while (stream) {
      auto &token = stream.token;

      switch (token.type) {

      case TokenType::IDENTIFIER: {
        if (token.content == "export") {
          ++stream;
          if (token.type != TokenType::EQUAL) {
            std::runtime_error("Expected '=' following 'export' keyword.");
          }
          ++stream;
          if (token.content == "true") {
            exported = true;
          } else if (token.content == "false") {
            exported = false;
          } else {
            std::runtime_error("Expected 'true' or 'false' value following "
                               "'export' keyword.");
          }
        }
        if (token.content == "name") {
          ++stream;
          if (token.type != TokenType::EQUAL) {
            std::runtime_error("Expected '=' following 'export' keyword.");
          }
          ++stream;
          if (token.type != TokenType::STRING) {
              std::runtime_error("Expected a string following 'name' keyword.");
          }
          name = token.content;
        }
      } break;
      default:
        break;
      }
      ++stream;
    }
  }
};

/** Parser for pxx export directives.
 *
 * This class parses comments of an C++ AST node into a vector of export
 * setting structs.
 */
class Parser {

public:
  Parser(const std::string &comments) : comments_(comments) {}

  /** Parse export settings in comment.
   * @return A vector containing an export setting struct for each
   * export directive in the string.
   */
  std::vector<ExportSettings> parse() {

    using StringIterator = decltype(comments_.begin());
    std::regex_iterator<StringIterator> directive_iterator(
        comments_.begin(), comments_.end(), pxx_directive);
    std::regex_iterator<StringIterator> end;

    std::vector<ExportSettings> results;

    while (directive_iterator != end) {
        //  auto directive = directive_iterator->
        auto directive_input = directive_iterator->operator[](1);
        TokenStream stream = TokenStream(directive_input);
        while (stream.token.type != TokenType::END) {
            results.emplace_back(stream);
        }
      ++directive_iterator;
    }
    return results;
  }

private:
  const std::string &comments_;
};

} // namespace directives
} // namespace pxx

#endif
