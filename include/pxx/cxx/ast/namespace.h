/** \file pxx/cxx/ast/namespace.h
 *
 * This file defines the Namespace class, which represents C++
 * namespaces in the AST>
 */
#ifndef __PXX_CXX_AST_NAMESPACE_H__
#define __PXX_CXX_AST_NAMESPACE_H__

namespace pxx {
namespace cxx {

#include <pxx/cxx/ast/ast_node.h>

////////////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////////////

class Namespace : public ASTNode {
public:
  Namespace(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, ASTNodeType::NAMESPACE, parent, scope) {}

private:
};

} // namespace cxx
} // namespace pxx
#endif
