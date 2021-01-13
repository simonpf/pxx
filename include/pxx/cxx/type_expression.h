/** \file pxx/cxx/type_expression.h
 *
 * Defines the TypeExpression class for manipulating expression involving
 * types.
 */
#include <string>
#include <locale>
#include <iostream>

#include <pxx/cxx/common.h>

namespace pxx {
namespace cxx {
namespace types {

namespace detail {
    inline bool is_identifier_char(char c) { return c == '_' || std::isalnum(c) ; }
    inline bool is_whitespace(char c) { return std::isblank(c); }
}

enum class IdentifierType {
  type_name,
  template_name,
  qualifier,
  end
};

////////////////////////////////////////////////////////////////////////////////
// Identifier iterator
////////////////////////////////////////////////////////////////////////////////

class IdentifierIterator {
public:
  IdentifierIterator(std::string &spelling)
      : spelling_(spelling), position(0), token_length_(0) {
        parse_identifier();
  }

  void parse_identifier();

  bool operator==(const IdentifierIterator &other) {
    return (spelling_ == other.spelling_) && (position == other.position);
  }

  char consume() {
      char c = 0;
      if (position < spelling_.size() - 1) {
          ++position;
          c = spelling_[position];
      }
      return c;
  }

  char get_current_char() {
      char c = 0;
      if (position < spelling_.size()) {
          c = spelling_[position];
      }
      return c;
  }


  IdentifierIterator &operator++() { parse_identifier(); return *this;}

  IdentifierType get_type() const { return type_; }

  std::string get_identifier() { return std::string(spelling_, token_start_, token_length_); }

private:
  const std::string &spelling_;
  size_t position, token_start_, token_length_;
  IdentifierType type_;
};

void IdentifierIterator::parse_identifier() {

    token_length_ = 0;
    type_ = IdentifierType::end;

    char current_char = get_current_char();

    // Skip anything not part of an identifier.
    while (!detail::is_identifier_char(current_char)) {
        current_char = consume();
    }

    // Did we reach the end?
    if (current_char == 0) {
        return;
    }

    token_start_ = position;
    // Parse identifier sequence.
    while (detail::is_identifier_char(current_char)) {
        current_char = consume();
        ++token_length_;
    }

    // Determine type
    if (current_char == ':') {
        type_ = IdentifierType::qualifier;
    } else if (current_char ==  '<') {
        type_ = IdentifierType::template_name;
    } else {
        type_ = IdentifierType::type_name;
    }

    if (is_keyword(get_identifier()))  {
        parse_identifier();
    }
}

//class TypeExpression {
//public:
//  TypeExpression(std::string spelling) : spelling_(spelling) {}
//
//  TypeExpressionToken get_next(size_t starting_position) {
//
//    TypeExpressionToken token;
//    size_t current_position = starting_position;
//
//    // Skip whitespace.
//    while (detail::is_whitespace(spelling_(current_position))) {
//      ++current_position;
//    }
//
//    // Start extracting the token text.
//    size_t start_of_identifier = current_position;
//    token.start = current_position;
//    while (detail::is_identifier_char(spelling(current_position))) {
//      current_position++;
//    }
//    token.length = current_position - start_of_identifier;
//    if (current_position == spelling_.size()) {
//      return TypeExpressionToken(spelling_,
//                                 start_of_identifier,
//                                 current_position - start_of_identifier,
//          )
//    }
//  }
//
//}
//
//std::string spelling_;
//};

}
} // namespace cxx
} // namespace pxx
