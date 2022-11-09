/** \file pxx/cxx/type_expression.h
 *
 * Defines the TypeExpression class for manipulating expression involving
 * types.
 */
#ifndef __PXX_CXX_TYPE_EXPRESSION_H__
#define __PXX_CXX_TYPE_EXPRESSION_H__

#include <iostream>
#include <locale>
#include <string>

#include <pxx/cxx/common.h>
#include <pxx/cxx/scope.h>
#include <pxx/cxx/ast.h>

namespace pxx {
namespace cxx {
namespace types {

namespace detail {
/// Whether a given char can make up an identifier.
inline bool is_identifier_char(char c) { return c == '_' || std::isalnum(c); }
/// Whether a char is a whitespace.
inline bool is_whitespace(char c) { return std::isblank(c); }
} // namespace detail

/// Type of identifiers.
enum class IdentifierType { type_name, template_name, qualifier, end };

////////////////////////////////////////////////////////////////////////////////
// Identifier iterator
////////////////////////////////////////////////////////////////////////////////

/** Iterate over identifiers in type expressions.
 *
 * This class implements an iterator class to iterate over identifiers in
 * C++ type expressions and to replace them if required.
 */
class IdentifierIterator {

  /// Parses next identifier in string.
  inline void parse_identifier();

  /// Extract char and advance reading position.
  char consume() {
    char c = 0;
    ++position_;
    if (position_ < (spelling_->size())) {
      c = (*spelling_)[position_];
    }
    return c;
  }

  /// Return current char.
  char get_current_char() {
    char c = 0;
    if (position_ < spelling_->size()) {
      c = (*spelling_)[position_];
    }
    return c;
  }

public:
  /** Create IdentifierIterator to iterate over type expression.
   *
   * @param spelling The type expression given as string.
   */
  IdentifierIterator(std::string &spelling)
      : spelling_(&spelling), position_(0), token_length_(0) {
    parse_identifier();
  }

  /** Two iterators are the same if the refer to the same expression
   * and are at the same position.
   */
  bool operator==(const IdentifierIterator &other) {
    return (spelling_ == other.spelling_) && (position_ == other.position_);
  }

  /// Move to next identifier.
  IdentifierIterator &operator++() {
    parse_identifier();
    return *this;
  }
  /// Return the type of the identifier.
  IdentifierType get_type() const { return type_; }
  /// Return the current identifier as string.
  std::string get_identifier() {
    return std::string(*spelling_, token_start_, token_length_);
  }
  /// Check if iterator is exhausted.
  operator bool() const { return type_ != IdentifierType::end; }

  std::string operator*() {
      return std::string(*spelling_, token_start_, token_length_);
  }

  /// Start position of current token.
  size_t get_token_start() {
      return token_start_;
  }

  /// Length of current token.
  size_t get_token_length() {
      return token_length_;
  }

  void operator=(std::string replacement) {
    spelling_->replace(token_start_, token_length_, replacement);
    position_ = token_start_ + replacement.size();
    token_length_ = replacement.size();
  }

  void replace(size_t start, size_t length, std::string replacement) {
    spelling_->replace(start, length, replacement);
    position_ = token_start_ + replacement.size();
    token_length_ = replacement.size();
  }

private:
  std::string *spelling_;
  size_t position_, token_length_, token_start_;
  IdentifierType type_;
};

inline void IdentifierIterator::parse_identifier() {


  token_length_ = 0;
  type_ = IdentifierType::end;

  char current_char = get_current_char();
  // Skip anything not part of an identifier.
  while (!detail::is_identifier_char(current_char) && (current_char != 0)) {
    current_char = consume();
  }

  // Did we reach the end?
  if (current_char == 0) {
    return;
  }

  token_start_ = position_;
  // Parse identifier sequence.
  while (detail::is_identifier_char(current_char)) {
    current_char = consume();
    ++token_length_;
  }

  // Determine type
  if (current_char == ':') {
    type_ = IdentifierType::qualifier;
  } else if (current_char == '<') {
    type_ = IdentifierType::template_name;
  } else {
    type_ = IdentifierType::type_name;
  }

  if (is_keyword(get_identifier())) {
    parse_identifier();
  }
  current_char = get_current_char();

  // Skip ahead to beginning of next identifier.
  while ((!detail::is_identifier_char(current_char)) && (current_char != 0)) {
    current_char = consume();
  }
}

inline std::string replace_type_names(std::string spelling, Scope *scope) {
  std::string result = spelling;
  types::IdentifierIterator iterator(result);
  std::string identifier = "";

  size_t start = 0;

  do {
    auto type = iterator.get_type();
    switch (type) {
    case types::IdentifierType::qualifier: {
      if (identifier != "") {
        identifier = identifier + "::" + *iterator;
      } else {
        identifier = *iterator;
        start = iterator.get_token_start();
      }
    } break;
    case types::IdentifierType::template_name:
    case types::IdentifierType::type_name: {
      if (identifier != "") {
        identifier = identifier + "::" + *iterator;
      } else {
        start = iterator.get_token_start();
        identifier = *iterator;
      }
      auto symbol = scope->lookup_symbol(identifier);
      if (!symbol) {
          symbol = scope->get_root_scope()->lookup_symbol(identifier);
      }
      std::cout << symbol << std::endl;
      if (symbol) {
          iterator.replace(start, identifier.size(), symbol->get_qualified_name());
      }
      identifier = "";
    } break;
    default:
        break;
    }
  } while (++iterator);
  return result;
}

} // namespace types
} // namespace cxx
} // namespace pxx

#endif
