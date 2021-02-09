/** \file pxx/cxx/ast/template.h
 *
 * This file defines the Template AST node class, which is used to represent
 * class and function templates.
 */
#ifndef __PXX_CXX_AST_TEMPLATE_H__
#define __PXX_CXX_AST_TEMPLATE_H__

#include <iostream>
#include <map>

#include <pxx/cxx/ast/ast_node.h>

namespace pxx {
namespace cxx {

/** Base class for templates.
 *
 * The template class represents a defition of a class or function template.
 */
class Template : public ASTNode {

public:

  /** Create new template object.
   * @param cursor The cursor representing the defition.
   * @param type The AST node type of the template.
   * @param parent The parent node of the template.
   * @param scope The scope in which the template is defined.
   */
  Template(CXCursor cursor, ASTNodeType type, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, type, parent, scope) {}

    /** Add a template parameter to the template.
     * @param cursor The libclang cursor representing the parameter declaration.
     */
  void add_template_parameter(CXCursor cursor) {
    parameters_.push_back(detail::get_name(cursor));
  }

  /** Add a specialization to the template.
   *
   * @param node The function or class template representing the specialization.
   */
  void add_specialization(std::unique_ptr<ASTNode> node) {
    specializations_.emplace(node->get_cursor_hash(), std::move(node));
  }

  /// Return map containing the specializations.
  std::map<unsigned int, std::unique_ptr<ASTNode>> &get_specializations() {
    return specializations_;
  }

  /** Add a template instance to the template.
   *
   * @param node The AST node representing the template instantiation.
   */
  void add_instance(std::unique_ptr<ASTNode> node) {
    instances_.emplace_back(std::move(node));
  }

  /// The vector of instances of the template.
  std::vector<std::unique_ptr<ASTNode>>& get_instances() { return instances_; }

  /** Lookup the template definition corresponding to a given cursor.
   *
   * @param cursor The cursor of the template definition.
   */
  ASTNode *get_template(CXCursor cursor) {
    auto hash = clang_hashCursor(cursor);
    if (hash == get_cursor_hash()) {
      return this;
    }
    return specializations_[hash].get();
  }

private:
  std::vector<std::string> parameters_ = {};
  std::map<unsigned int, std::unique_ptr<ASTNode>> specializations_ = {};
  std::vector<std::unique_ptr<ASTNode>> instances_ = {};
};

} // namespace cxx
} // namespace pxx
#endif
