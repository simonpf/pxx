#include <pxx/cxx/ast/ast_node.h>

namepsace pxx {
namespace cxx {

class TypeDef : public ASTNode {
    TypeDef(CXCursor cursor, ASTNodeType node_type, ASTNode *parent, Scope *scope)
        : ASTNode(cursor, node_type, parent, scope) {
        name_ = to_string
        target_ = to_string(
            clang_getTypedefDeclUnderlyingType(cursor)
            );

        std::cout <<" TYPEDEF: " << name_ << " / " << target_;

    }

private:
    std::string name_;
    std::string target_;
};
}}
