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

class Template : public ASTNode {

public:
  Template(CXCursor cursor, ASTNodeType type, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, type, parent, scope) {}

  void add_template_parameter(CXCursor cursor) {
    parameters_.push_back(detail::get_name(cursor));
  }

  void add_specialization(CXCursor cursor, std::unique_ptr<ASTNode> node) {
    specializations_.emplace(clang_hashCursor(cursor), std::move(node));
  }

  ASTNode *get_template(CXCursor cursor) {
    auto hash = clang_hashCursor(cursor);
    if (hash == clang_hashCursor(cursor_)) {
      return this;
    }
    return specializations_[hash].get();
  }

private:
  std::vector<std::string> parameters_ = {};
  std::map<unsigned int, std::unique_ptr<ASTNode>> specializations_ = {};
};

} // namespace cxx
} // namespace pxx
#endif
