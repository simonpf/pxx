/** \file pxx/cxx/ast/class.h
 *
 * This file defines the class used to represent C++ class, constructors,
 * member functions and member variables.
 *
 */
#ifndef __PXX_CXX_AST_CLASS_H__
#define __PXX_CXX_AST_CLASS_H__

#include <pxx/clang.h>
#include <pxx/cxx/ast/ast_node.h>
#include <pxx/cxx/ast/template.h>
#include <pxx/cxx/type_expression.h>

namespace pxx {
namespace cxx {

enum class Access { PUBLIC, PRIVATE, PROTECTED };
namespace detail {
inline Access get_access_level(CXCursor cursor) {
  auto access_specifier = clang_getCXXAccessSpecifier(cursor);
  switch (access_specifier) {
  case CX_CXXPublic:
    return Access::PUBLIC;
    break;
  case CX_CXXProtected:
    return Access::PROTECTED;
    break;
  case CX_CXXPrivate:
    return Access::PRIVATE;
    break;
  default:
    break;
  }
  return Access::PRIVATE;
}

inline bool is_const(CXCursor cursor) {
    auto type = clang_getCursorType(cursor);
    return clang_isConstQualifiedType(type);
}

} // namespace detail


class ClassTemplate : public Template {
public:
ClassTemplate(CXCursor cursor, ASTNode *parent, Scope *scope)
    : Template(cursor, ASTNodeType::CLASS_TEMPLATE, parent, scope) {}
};

/** A C++ class.
 *
 * This ASTNode class represents the definition of a C++ class.
 */
class Class : public ASTNode {
public:


  Class(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, ASTNodeType::CLASS, parent, scope) {

    auto templ = clang_getSpecializedCursorTemplate(cursor);
    if (!clang_Cursor_isNull(templ)) {
      std::string qualified_name = pxx::clang::get_qualified_name(templ);
      auto tmpl = dynamic_cast<ClassTemplate*>(scope->lookup_symbol(qualified_name));
      template_ = dynamic_cast<ClassTemplate*>(tmpl->get_template(templ));
    }
  }

    const ClassTemplate* get_template() const { return template_; }

    void write_bindings(std::ostream& output) const override {
        auto qualified_name = get_qualified_name();
        output << "  py::class_<" << qualified_name << "> py_class";
        output << "{module, \"" << name_ << "\"};" << std::endl;
        for (auto &c : children_) {
            c->write_bindings(output);
        }
    }

private:
  ClassTemplate *template_ = nullptr;
};

/// A class member variable.
class MemberVariable : public ASTNode {
public:
  MemberVariable(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, ASTNodeType::MEMBER_VARIABLE, parent, scope),
        is_static_(clang_CXXMethod_isStatic(cursor)) {
    access_level_ = detail::get_access_level(cursor);
    std::cout << "MEMBERVAR :: " << name_ << std::endl;
  }

  void write_bindings(std::ostream &output) const override {
    auto qualified_name = get_qualified_name();
    if (access_level_ == Access::PUBLIC) {
      if (is_const_) {
        output << "  py_class.def_readonly(\"" << name_ << "\", &"
               << qualified_name;
        output << ");" << std::endl;
      } else {
        output << "  py_class.def_readwrite(\"" << name_ << "\", &"
               << qualified_name;
        output << ");" << std::endl;
      }
    }
  }

private:
  bool is_const_;
  bool is_static_;
  Access access_level_;
};

using types::replace_type_names;

/** Class member function.
 *
 * Specialization of Function class to take into account the
 * different spelling of function pointer types for general functions
 * and class member functions.
 */
class MemberFunction : public Function {


protected:
  MemberFunction(CXCursor cursor, ASTNodeType type, ASTNode *parent,
                 Scope *scope)
      : Function(cursor, type, parent, scope),
        is_const_(clang_CXXMethod_isConst(cursor)),
        is_static_(clang_CXXMethod_isStatic(cursor)) {
    }

public:
  MemberFunction(CXCursor cursor, ASTNode *parent, Scope *scope)
      : MemberFunction(cursor, ASTNodeType::MEMBER_FUNCTION, parent, scope) {
        access_level_ = detail::get_access_level(cursor);
    }

  inline friend std::ostream &operator<<(std::ostream &out,
                                         const MemberFunction &);

  std::string get_pointer_spelling() const {
      std::string parent_name = parent_->get_qualified_name();
      std::string return_type = replace_type_names(return_type_, scope_);
      std::string result = return_type + " (";
      if (!is_static_) result += parent_name + "::";
      result += "*)(";

      for (size_t i = 0; i < argument_types_.size(); ++i) {
          result += replace_type_names(argument_types_[i], scope_);
          if (i + 1 < argument_types_.size()) {
              result += ", ";
          }
      }
      result += ")";
      if (is_const_) result += " const";
      return result;
  }

  static ASTNodeType get_node_type() { return ASTNodeType::MEMBER_FUNCTION; }

  void write_bindings(std::ostream &output) const override {
    if (access_level_ == Access::PUBLIC) {
      auto qualified_name = get_qualified_name();
      auto pointer_type = get_pointer_spelling();
      if (comment_ == "") {
          output << "  py_class.def(\"" << name_ << "\", &" << qualified_name << ");";
      } else {
          output << "  py_class.def(\"" << name_ << "\", ";
          output << "static_cast<" << pointer_type << "> (&" << qualified_name << "),";
          output << std::endl << print_comment_as_raw_string() << ");";
      }
      output << std::endl;
  }
  }

private:
  bool is_const_;
  bool is_static_;
  Access access_level_;
};

inline std::ostream &operator<<(std::ostream &out,
                                const MemberFunction &method) {
  out << method.return_type_ << " ";
  out << method.parent_->get_qualified_name();
  out << "::" << method.get_name() << "(";
  for (size_t i = 0; i < method.argument_types_.size(); ++i) {
    out << method.argument_types_[i];
    if (i + 1 < method.argument_types_.size()) {
      out << ", ";
    }
  }
  out << ")";
  return out;
}

/** Class constructor.
 *
 * Specialization of the MemberFunction class for class constructors.
 *
 */
class Constructor : public MemberFunction {

  static const std::map<std::string, std::string> special_functions_;

public:
  Constructor(CXCursor cursor, ASTNode *parent, Scope *scope)
      : MemberFunction(cursor, ASTNodeType::CONSTRUCTOR, parent, scope) {

  }

  static ASTNodeType get_node_type() { return ASTNodeType::CONSTRUCTOR; }

  void write_bindings(std::ostream &output) const override {
      auto qualified_name = get_qualified_name();
      output << "  py_class.def("
             << "py::init<";
      for (size_t i = 0; i < argument_types_.size(); ++i) {
          std::string type = argument_types_[i];
          output << types::replace_type_names(type, scope_);
          if (i + 1 < argument_types_.size()) {
              output << ", ";
          }
      }
      output << ">());" << std::endl;
  }
};

} // namespace cxx
} // namespace pxx

#endif
