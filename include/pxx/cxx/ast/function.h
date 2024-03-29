/** \file pxx/cxx/ast/funcion.h
 *
 * This file defines the Function and Overload class, which are used to
 * represent C++ functions in the AST.
 */
#ifndef __PXX_CXX_AST_FUNCTION_H__
#define __PXX_CXX_AST_FUNCTION_H__

#include <pxx/cxx/ast/ast_node.h>
#include <pxx/cxx/ast/template.h>
#include <pxx/cxx/type_expression.h>

namespace pxx {
namespace cxx {

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

/** A specific function.
 *
 * This class represents a specific function identified by its name
 * and call signature.
 */
class Function : public ASTNode {
protected:
  // Protected constructor used by child classes.
  Function(CXCursor cursor, ASTNodeType node_type, ASTNode *parent,
           Scope *scope)
      : ASTNode(cursor, node_type, parent, scope) {
    auto type = clang_getCursorType(cursor);
    return_type_ = detail::get_type_spelling(clang_getResultType(type));

    int n_args = clang_getNumArgTypes(type);
    argument_types_.reserve(n_args);
    for (int i = 0; i < n_args; ++i) {
      argument_types_.emplace_back(
          detail::get_type_spelling(clang_getArgType(type, i)));
    }
  }

public:
  /** Create a new function
   * @params cursor libclang CXCursor object pointing to the function
   * definition.
   */
  Function(CXCursor cursor, ASTNode *parent, Scope *scope)
      : Function(cursor, ASTNodeType::FUNCTION, parent, scope) {}

  /// Static function to determine the node type of this class.
  static ASTNodeType get_node_type() { return ASTNodeType::FUNCTION; }

  std::string get_pointer_spelling() const {
      std::string parent_name = parent_->get_qualified_name();
      std::string return_type = types::replace_type_names(return_type_, scope_);
      std::string result = return_type + " (*)(";
      for (size_t i = 0; i < argument_types_.size(); ++i) {
          result += types::replace_type_names(argument_types_[i], scope_);
          if (i + 1 < argument_types_.size()) {
              result += ", ";
          }
      }
      result += ")";
      return result;
  }

  void write_bindings(std::ostream &output) const override {
      auto qualified_name = get_qualified_name();
      auto pointer_type = get_pointer_spelling();
      output << "module.def(\"" << name_ << "\", ";
      output << "static_cast<" << pointer_type << "> (&" << qualified_name << "));";
      output << std::endl;
  }

  inline friend std::ostream &operator<<(std::ostream &out, const Function &);

protected:
  std::string return_type_ = "";
  std::vector<std::string> argument_types_ = {};
};

class FunctionTemplate : public Template {
public:
    FunctionTemplate(CXCursor cursor,
                     ASTNode *parent,
                     Scope *scope)
        :
    Template(cursor, ASTNodeType::FUNCTION_TEMPLATE, parent, scope)
         {}
    /// Static function to determine the node type of this class.
    static ASTNodeType get_node_type() { return ASTNodeType::FUNCTION_TEMPLATE; }
};

inline std::ostream &operator<<(std::ostream &out, const Function &function) {
  out << function.return_type_ << " ()(";
  for (size_t i = 0; i < function.argument_types_.size(); ++i) {
    out << function.argument_types_[i];
    if (i + 1 < function.argument_types_.size()) {
      out << ", ";
    }
  }
  out << ")";
  return out;
}

/** Generic class to represent overloaded functions and methods.
 *
 * This class represents a function of class method identified solely by its
 * name. Since there could exist several overloads for the function, the
 * Overload class represents the generic case of named function with one or
 * more overloads.
 *
 * @tparam FunctionClass The class representing the type of the underlying
 * overloaded functions.
 */
template <typename FunctionClass> class Overload : public ASTNode {
public:
  /** Create FunctionOverload from cursor.
   *
   * @param cursor libclang CXCursor of kind CXCursor_FunctionDecl or similar.
   * @param parent Pointer to parent object.
   * @param scope Pointer to the scope that the object lives in.
   */
  Overload(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, FunctionClass::get_node_type(), parent, scope) {}

  /// The number of overloads.
  size_t get_n_overloads() { return functions_.size(); }

  /// Return vector of overloads.
  std::vector<std::unique_ptr<FunctionClass>>& get_overloads() { return functions_; }

  /// Add an overload to the function name.
  FunctionClass* add_overload(CXCursor cursor) {
    functions_.emplace_back(std::make_unique<FunctionClass>(cursor, parent_, scope_));
    return functions_.back().get();
  }

  virtual void print_tree(std::ostream &out, size_t indent = 2,
                          size_t offset = 0) const {
    out << std::setw(offset) << "";
    out << "Overload: " << name_ << "(" << source_file_.filename() << ")"
        << std::endl;
    for (auto &f : functions_) {
      out << std::setw(offset + indent) << "";
      out << f << std::endl;
    }
  }

protected:
  std::vector<std::unique_ptr<FunctionClass>> functions_;
};

} // namespace cxx
} // namespace pxx
#endif
