/** \file type_parser.h
 *
 * This file implements a parse for C++ type names.
 *
 * Author: Simon Pfreundschuh, 2021
 */
#ifndef __PXX_TYPE_PARSER_H__
#define __PXX_TYPE_PARSER_H__

#include <vector>
#include <string>
#include <iostream>
#include <memory>

namespace pxx {
namespace types {

////////////////////////////////////////////////////////////////////////////////
// Representation of types.
////////////////////////////////////////////////////////////////////////////////

class TypeBase {
public:
  TypeBase(std::vector<std::string> qualifiers) : qualifiers_(qualifiers) {}

    virtual void print(std::ostream &out) const = 0;

protected:
    std::vector<std::string> qualifiers_;
};

std::ostream & operator<<(std::ostream &out, const TypeBase &t) {
    t.print(out);
    return out;
}

using TypePtr = std::unique_ptr<TypeBase>;

//
// Basic type
//

/** A fundamental or user-defined type. */
class Type : public TypeBase {

public:
  Type(std::string scope_name, std::string type_name,
       std::vector<std::string> qualifiers)
      : TypeBase(qualifiers), scope_name_(scope_name), type_name_(type_name) {}

  Type(std::string type_name, std::vector<std::string> qualifiers)
      : TypeBase(qualifiers), scope_name_(""), type_name_(type_name) {}

  Type(std::string type_name)
      : TypeBase({}), scope_name_(""), type_name_(type_name) {}

  Type(const Type &) = default;

  void print(std::ostream &out) const {
    if (scope_name_.size() > 0) {
      out << scope_name_ << "::";
    }
    out << type_name_;
    for (size_t i = 0; i < qualifiers_.size(); ++i) {
        out << " " << qualifiers_[i];
    }
  }

protected:
  std::string scope_name_;
  std::string type_name_;
};

//
// Template type
//

/** A class template with arguments. */
class TypeTemplate : public Type {
public:
  TypeTemplate(std::string scope_name,
               std::string type_name,
               std::vector<std::string> qualifiers,
               std::vector<TypePtr> &arguments)
      : Type(scope_name, type_name, qualifiers), arguments_(std::move(arguments)) {}

  void print(std::ostream &out) const {
    if (scope_name_.size() > 0) {
      out << scope_name_ << "::";
    }
    out << type_name_ << "<";
    for (size_t i = 0; i < arguments_.size(); ++i) {
      out << *arguments_[i];
      if (i < arguments_.size() - 1) {
        out << ", ";
      }
    }
    out << ">";
    for (size_t i = 0; i < qualifiers_.size(); ++i) {
      out << " " << qualifiers_[i];
    }
  }

private:
  std::vector<TypePtr> arguments_;
};


//
// A function pointer type
//

/** A function type */
class FunctionPointerType : public TypeBase {

public:

FunctionPointerType(TypePtr &return_type,
                    std::vector<TypePtr> &arguments)
    : TypeBase({}), arguments_(std::move(arguments)), return_type_(std::move(return_type)) {}

  void print(std::ostream &out) const {
    out << *return_type_ << " (*)(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
      out << *arguments_[i];
      if (i < arguments_.size() - 1) {
        out << ", ";
      }
    }
    out << ")";
  }

private:
  std::vector<TypePtr> arguments_;
  TypePtr return_type_;
};

//
// A member pointer type.
//

class MemberPointerType : public TypeBase {

public:
  MemberPointerType(TypePtr &type, TypePtr &parent)
      : TypeBase({}), type_(std::move(type)), parent_(std::move(parent)) {}

  void print(std::ostream &out) const { out << *type_ << " " << *parent_ << "::*"; }

private:
  TypePtr type_;
  TypePtr parent_;
};

////////////////////////////////////////////////////////////////////////////////
// Source
////////////////////////////////////////////////////////////////////////////////

class Source {

public:
  static constexpr char EOL = '\n';
  Source(std::string source)
      : length_(source.size()), position_(0), source_(source) {
      std::cout << "source: " << source_.size() << std::endl;

  }

  char current_char() {
    if (position_ == 0) {
      return next_char();
    }
    if (position_ > length_) {
      return EOL;
    }
    return source_[position_ - 1];
  }

  char next_char() {
    ++position_;
    return current_char();
  }

  char peek_char() {
    size_t next_position = position_ + 1;
    if (next_position > length_) {
      return EOL;
    }
    return source_[next_position - 1];
  }

  template <size_t N> std::array<char, N> peek() {
    std::array<char, N> result;
    size_t position = position_;
    for (size_t i = 0; i < N; ++i) {
      if (position > length_) {
        result[i] = EOL;
      } else {
          result[i] = source_[position - 1];
      }
      ++position;
    }
    return result;
  }

private:
  size_t length_, position_;
  std::string source_;
};

enum class TokenType {
  PointerQualifier,
  ReferenceQualifier,
  ConstQuanlifier,
  VolatileQualifier,
  TypeName
};

class Token {


    TokenType type;
    std::string text;
};

class Scanner {
public:
Scanner(Source source) : source_(source) {}

private:
    Source source_;
};

////////////////////////////////////////////////////////////////////////////////
// Parser for types.
////////////////////////////////////////////////////////////////////////////////

class Parser {

public:
private:
};

} // namespace types
} // namespace pxx

#endif
