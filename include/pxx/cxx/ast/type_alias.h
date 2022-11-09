/** \file pxx/cxx/ast/type_alias.h
 *
 * This file defines the TypeAlias class, which represents non-template type
 * aliases in C++.
 */
#include <pxx/cxx/ast/ast_node.h>
#include <pxx/cxx/type_expression.h>

namespace pxx {
namespace cxx {

class TypeAlias : public ASTNode {
 public:
  TypeAlias(CXCursor cursor, ASTNode *parent, Scope *scope)
      : ASTNode(cursor, ASTNodeType::TYPE_ALIAS, parent, scope) {
        name_ = to_string(clang_getCursorSpelling(cursor));
      auto target_type = clang_getTypedefDeclUnderlyingType(cursor);
    target_type_ = to_string(clang_getTypeSpelling(target_type));


  }


  std::string get_qualified_name() const override {
      std::cout << "QUALIFIED NAME :: " << target_type_ << std::endl;
      return replace_type_names(target_type_, scope_);
  }

 private:
  std::string name_;
  std::string target_type_;
};
}  // namespace cxx
}  // namespace pxx
