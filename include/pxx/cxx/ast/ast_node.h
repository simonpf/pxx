/** \file pxx/cxx/ast/ast_node.h
 *
 * This file contains the basic functions and types used in the C++
 * AST and defines the base class for AST nodes.
 */
#ifndef __PXX_CXX_AST_AST_NODE_H__
#define __PXX_CXX_AST_AST_NODE_H__

#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <tuple>

#include <pxx/clang.h>
#include <pxx/cxx/scope.h>
#include <pxx/utils.h>

namespace pxx {
namespace cxx {

using pxx::to_string;
namespace detail {

    inline std::string get_name(CXCursor c) {
        CXString s = clang_getCursorSpelling(c);
        return pxx::to_string(s);
    }

    inline std::string get_type_spelling(CXType t) {
        CXString s = clang_getTypeSpelling(t);
        return pxx::to_string(s);
    }

    std::tuple<std::filesystem::path, size_t, size_t> inline get_cursor_location(
        CXCursor c) {
        CXFile file;
        unsigned line_number, column, offset;
        auto location = clang_getCursorLocation(c);
        clang_getSpellingLocation(location, &file, &line_number, &column, &offset);
        std::string file_name = pxx::to_string(clang_getFileName(file));
        return std::make_tuple(file_name, line_number, column);
    }
} // namespace detail

enum class ASTNodeType {
  ROOT,
  NAMESPACE,
  FUNCTION,
  CLASS,
  MEMBER_FUNCTION,
  MEMBER_VARIABLE,
  CONSTRUCTOR,
  CLASS_TEMPLATE,
  FUNCTION_TEMPLATE,
  TYPE_ALIAS,
  TYPE_DECLARATION,
  TEMPLATE_TYPE_ALIAS,
  UNDEFINED
};

enum class Accessibility { PUBLIC, PRIVATE, PROTECTED };

namespace detail {
/** Determine accessibility of cursor and convert to
 *  Accessiblity enum.
 *
 * @param cursor libclang CXCursor referring to an C++ AST node
 * whose access level should be determined.
 * @return Accessiblity enum representing the AST nodes accessiblity.
 */
inline Accessibility get_accessibility(CXCursor cursor) {
  CX_CXXAccessSpecifier as = clang_getCXXAccessSpecifier(cursor);
  if (as == CX_CXXPublic) {
    return Accessibility::PUBLIC;
  } else if (as == CX_CXXProtected) {
    return Accessibility::PROTECTED;
  } else if (as == CX_CXXPrivate) {
    return Accessibility::PRIVATE;
  }
  return Accessibility::PUBLIC;
}
} // namespace detail

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
  /** Create new AST node.
   *
   * @param The libclang cursor defining the node.
   * @param type The type of the AST node.
   * @param parent Pointer to the parent node of the newly created node.
   * @param scope Pointer to the scope in which the AST node is defined.
   */
  ASTNode(CXCursor cursor, ASTNodeType type, ASTNode *parent, Scope *scope)
      : type_(type), access_(detail::get_accessibility(cursor)),
        parent_(parent), cursor_(cursor), cursor_hash_(clang_hashCursor(cursor)),
        scope_(scope) {
    name_ = detail::get_name(cursor);
    auto location = detail::get_cursor_location(cursor);
    source_file_ = std::get<0>(location);
    line_ = std::get<1>(location);
    column_ = std::get<2>(location);
  }

  /// The name of the node.
  const std::string &get_name() const { return name_; }

  /// Return libclang cursor.
  CXCursor get_cursor() const { return cursor_; }

  /// Return cursor hash.
  unsigned int get_cursor_hash() const { return cursor_hash_; }

  /// The type of the node.
  ASTNodeType get_type() { return type_; }

  /// The visibility of the node: public, protected or private.
  Accessibility get_accessibility() { return access_; }

  /// The scope in which the symbol is defined.
  Scope *get_scope() { return scope_; }

  /// Return childrent map.
  const std::map<std::string, ASTNode *> &get_children() const {
    return children_;
  }

  /// Add child to node.
  ASTNode *add_child(ASTNode *child) {
    auto result = children_.emplace(child->get_name(), child);
    return result.first->second;
  }

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
  virtual void print_tree(std::ostream &out, size_t indent = 2,
                          size_t offset = 0) const {
    out << std::setw(offset) << "";
    out << "ASTNode: " << name_ << "(" << source_file_.filename() << ")"
        << std::endl;
    for (auto &c : children_) {
      c.second->print_tree(out, indent, offset + indent);
    }
  }

  /** Return qualified name of AST node.
   * @return The name of the node that identifies it at the root scope.
   */
  virtual std::string get_qualified_name() {
    return scope_->get_prefix() + name_;
  }

  //
  // Friends
  //

  friend std::ostream &operator<<(std::ostream &out, const ASTNode &node);

protected:
  ASTNodeType type_;
  Accessibility access_;
  ASTNode *parent_;
  CXCursor cursor_;
  unsigned int cursor_hash_;
  Scope *scope_;
  std::string name_;
  std::map<std::string, ASTNode *> children_ = {};
  std::filesystem::path source_file_;
  size_t line_, column_;
};

inline std::ostream &operator<<(std::ostream &out, const ASTNode &node) {
  node.print_tree(out, 2, 0);
  return out;
}

} // namespace cxx
} // namespace pxx

#endif
