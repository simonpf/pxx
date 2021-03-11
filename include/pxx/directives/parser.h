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
    std::regex pxx_directive { "^[ \t]*//[ \t]*pxx[ \t]*::" };

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
      do {
        previous = c;
        c = input.consume();
        ++length;
      } while ((previous != '\\') && (c != '\"'));
      content = input.extract(start + 1, length - 1);
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

class Lexer {

    Lexer(std::string input) : input_(input) {}


    Token next() {
        Input input{input_.str()};
        return Token(input);
    }


private:

    std::stringstream input_;

};



class Parser {};

} // namespace directives
} // namespace pxx

#endif
