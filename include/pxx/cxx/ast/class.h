/** \file pxx/cxx/ast/class.h
 *
 * This file defines the class used to represent C++ class, constructors,
 * member functions and member variables.
 *
 */
#ifndef __PXX_CXX_AST_CLASS_H__
#define __PXX_CXX_AST_CLASS_H__

#include <pxx/cxx/ast/ast_node.h>

namespace pxx {
namespace cxx {

/** A C++ class.
 *
 * This ASTNode class represents the definition of a C++ class.
 */
class Class : public ASTNode {
public:
  Class(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, ASTNodeType::CLASS, parent, scope) {}

private:
};

class MemberVariable : public ASTNode {
public:
  MemberVariable(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, ASTNodeType::MEMBER_VARIABLE, parent, scope) {}

private:
};

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
        is_static_(clang_CXXMethod_isStatic(cursor)) {}

public:
  MemberFunction(CXCursor cursor, ASTNode *parent, Scope *scope)
      : MemberFunction(cursor, ASTNodeType::MEMBER_FUNCTION, parent, scope) {}

  inline friend std::ostream &operator<<(std::ostream &out,
                                         const MemberFunction &);

  static ASTNodeType get_node_type() { return ASTNodeType::MEMBER_FUNCTION; }

private:
  bool is_const_;
  bool is_static_;
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
      : MemberFunction(cursor, ASTNodeType::CONSTRUCTOR, parent, scope) {}

  static ASTNodeType get_node_type() { return ASTNodeType::CONSTRUCTOR; }
};

} // namespace cxx
} // namespace pxx

#endif
