/** \file pxx/cxx/ast.h
 *
 * This file defines the pxx AST, which is used to represent the namespaces, classes
 * and functions parsed using libclang.
 */
#ifndef __PXX_CXX_AST_H__
#define __PXX_CXX_AST_H__

#include <filesystem>
#include <iostream>
#include <memory>
#include <tuple>
#include <pxx/utils.h>

namespace pxx {
namespace cxx {

using pxx::to_string;
namespace detail {
inline std::string get_name(CXCursor c) {
  CXString s = clang_getCursorSpelling(c);
  return pxx::to_string(s);
}

std::tuple<std::filesystem::path, size_t, size_t>
inline get_cursor_location(CXCursor c) {
  CXFile file;
  unsigned line_number, column, offset;
  auto location = clang_getCursorLocation(c);
  clang_getSpellingLocation(location, &file, &line_number, &column, &offset);
  std::string file_name = pxx::to_string(clang_getFileName(file));
  return std::make_tuple(file_name, line_number, column);
}

} // namespace detail

/** Scope class to keep track of defined names.
 *
 * The Scope class handles the different name spaces in order to
 * be able to determine the qualified name of types, class and
 * functions.
 */
class Scope {

public:

    /** Create a new scope.
     * @param name The name of the new scope.
     * @param parent Pointer to parent scope or nullptr if root scope.
     */
  Scope(std::string name, Scope *parent) : name_(name), parent_(parent) {}


    /** Add child scope.
     *
     * Adds a child scope with the given name to this scope.
     * @param name The name of the child scope to add.
     * @return Pointer to the created child scope.
     */
  Scope *add_child(std::string name) {
    children_.emplace_back(std::make_unique<Scope>(name, this));
    return children_.back().get();
  }

  /** Return scope prefix.
   *
   * Get the qualifying prefix that identifies this scope.
   */
  std::string get_prefix() {
    std::string prefix = "";
    if (parent_) {
      prefix = parent_->get_prefix() + "::";
    }
    return prefix + "::" + name_;
  }

private:
  std::string name_;
  Scope *parent_;
  std::vector<std::unique_ptr<Scope>> children_ = {};
};

enum class ASTNodeType {
  ROOT,
  NAMESPACE,
  FUNCTION,
  CLASS,
  MEMBER_FUNCTION,
  CONSTRUCTOR,
  CLASS_TEMPLATE,
  FUNCTION_TEMPLATE,
  TYPE_ALIAS,
  TYPE_DECLARATION,
  TEMPLATE_TYPE_ALIAS,
  UNDEFINED
};

////////////////////////////////////////////////////////////////////////////////
// AST Node base class
////////////////////////////////////////////////////////////////////////////////

/** Base class for all AST nodes.
 *
 * The ASTNode class is the base class for all AST nodes and holds the basic
 * information that applies to every parsed language object.
 */
class ASTNode {

public:
  ASTNode(CXCursor cursor, ASTNodeType type, ASTNode *parent, Scope *scope)
      : type_(type), parent_(parent), scope_(scope) {
    name_ = detail::get_name(cursor);

    auto location = detail::get_cursor_location(cursor);
    source_file_ = std::get<0>(location);
    line_ = std::get<1>(location);
    column_ = std::get<2>(location);

  }


  void add_child(std::unique_ptr<ASTNode> &&child) {
      children_.emplace_back(std::move(child));
  }

  const std::string &get_name() const {return name_;}

  friend std::ostream &operator<<(std::ostream &out, const ASTNode &node);

  //
  // Virtual functions.
  //

  /** Print AST tree.
   *
   * This prints a textual representation of the AST that this is
   * the root of.
   *
   * @param out The outpustream to print to.
   * @param indent The amount of spaces with which to indent the
   * child nodes of this node relative to this node's indentation.
   * @param offset The amount of spaces by which to indent the root
   * node.
   */
  virtual void print_tree(std::ostream &out,
                          size_t indent = 2,
                          size_t offset = 0) const {
    out << std::setw(offset) << "";
    out << "ASTNode: " << name_ << "(" << source_file_.filename() << ")"
        << std::endl;
    for (auto &c : children_) {
      c->print_tree(out, indent, offset + indent);
    }
  }

  /** Return qualified name of AST node.
   * @return The name of the node that identifies it at the root scope.
   */
  virtual std::string get_qualified_name() {
      auto prefix = scope_->get_prefix();
      return prefix + "::" + name_;
  }

protected:
  ASTNodeType type_;
  ASTNode *parent_;
  Scope *scope_;
  std::string name_;
  std::vector<std::unique_ptr<ASTNode>> children_ = {};
  std::filesystem::path source_file_;
  size_t line_, column_;
};

inline std::ostream &operator<<(std::ostream &out, const ASTNode &node) {
    node.print_tree(out, 2, 0);
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Class
////////////////////////////////////////////////////////////////////////////////


class NameSpace : public ASTNode {
public:

    NameSpace(CXCursor cursor,
              ASTNode *parent,
              Scope *scope)
        : ASTNode(cursor, ASTNodeType::CLASS, parent, scope) {}



private:


};

class Class : public ASTNode {
public:


private:

};


} // namespace cxx
} // namespace pxx

#endif
