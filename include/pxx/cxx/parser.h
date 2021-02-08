/** \file pxx/cxx/parser.h
 *
 * This file defines the parser class, which parser the libclang AST and
 * builds the pxx AST and scope.
 *
 */
#ifndef __PXX_CXX_PARSER_H__
#define __PXX_CXX_PARSER_H__

#include <clang-c/Index.h>
#include <memory>
#include <pxx/cxx/ast.h>
#include <string>
#include <tuple>

namespace pxx {
namespace cxx {

namespace {
CXChildVisitResult parse_clang_ast(CXCursor cursor, CXCursor /*parent*/,
                                   CXClientData client_data) {
  auto data = reinterpret_cast<std::tuple<ASTNode *, Scope *> *>(client_data);
  ASTNode *parent = std::get<0>(*data);
  Scope *scope = std::get<1>(*data);

  CXCursorKind k = clang_getCursorKind(cursor);
  switch (k) {

  // Namespace declaration.
  case CXCursor_ClassDecl: {
    auto child = scope->add<Class>(cursor, parent);
    auto new_scope = scope->add_child_scope(child->get_name());
    auto data = std::make_tuple(child, new_scope);
    clang_visitChildren(cursor, parse_clang_ast, &data);
    parent->add_child(child);
  } break;

  //
  // Classes
  //

  // Class declaration.
  case CXCursor_Namespace: {
    auto child = scope->add<Namespace>(cursor, parent);
    auto new_scope = scope->add_child_scope(child->get_name());
    auto data = std::make_tuple(child, new_scope);
    clang_visitChildren(cursor, parse_clang_ast, &data);
    parent->add_child(std::move(child));
  } break;

  // Class constructor.
  case CXCursor_Constructor: {
      auto child = dynamic_cast<Overload<Constructor>*>(
          scope->add<Overload<Constructor>>(cursor, parent)
          );
      child->add_overload(cursor);
      parent->add_child(child);
  } break;

  // Class member declaration.
  case CXCursor_CXXMethod: {
      auto child = dynamic_cast<Overload<MemberFunction>*>(
          scope->add<Overload<MemberFunction>>(cursor, parent)
          );
    child->add_overload(cursor);
    parent->add_child(child);
  } break;

  // Class member variable.
  case CXCursor_FieldDecl: {
      auto child = scope->add<MemberVariable>(cursor, parent);
      parent->add_child(child);
  } break;

  // Function definition.
  case CXCursor_FunctionDecl: {
      auto child = dynamic_cast<Overload<Function>*>(
          scope->add<Overload<Function>>(cursor, parent)
          );
    child->add_overload(cursor);
    parent->add_child(child);
  } break;

  //
  // Templates
  //

  case CXCursor_ClassTemplate: {
      auto child = scope->add<ClassTemplate>(cursor, parent);
      auto data = std::make_tuple(child, scope);
      clang_visitChildren(cursor, parse_clang_ast, &data);
      parent->add_child(child);
  } break;

  case CXCursor_ClassTemplatePartialSpecialization: {
      std::string qualified_name = pxx::clang::get_qualified_name(cursor);
      auto templ = dynamic_cast<ClassTemplate*>(scope->lookup_symbol(qualified_name));
      auto new_class = std::make_unique<Class>(cursor, parent, scope);
      templ->add_specialization(cursor, std::move(new_class));
  } break;

  case CXCursor_FunctionTemplate: {
      auto child = dynamic_cast<Overload<FunctionTemplate>*>(
          scope->add<Overload<FunctionTemplate>>(cursor, parent)
          );
      auto function = child->add_overload(cursor);
      auto data = std::make_tuple(function, scope);
      clang_visitChildren(cursor, parse_clang_ast, &data);
      parent->add_child(child);
  } break;

  case CXCursor_TemplateTypeParameter: {
      auto t = reinterpret_cast<Template*>(parent);
      t->add_template_parameter(cursor);
  } break;

  case CXCursor_NonTypeTemplateParameter: {
      auto t = reinterpret_cast<Template*>(parent);
      t->add_template_parameter(cursor);
  } break;


  default:
    break;
  }
  return CXChildVisit_Continue;
}
} // namespace

////////////////////////////////////////////////////////////////////////////////
// Parser class
////////////////////////////////////////////////////////////////////////////////

class Parser {
public:
  Parser(std::filesystem::path filename,
         std::vector<std::string> additional_args = {}) {
    std::vector<const char *> command_line_args = {"-x", "c++", "-std=c++11",
                                                   "-fparse-all-comments"};
    for (auto &s : additional_args) {
      command_line_args.push_back(s.c_str());
    }
    index_ = clang_createIndex(0, 0);
    unit_ = clang_parseTranslationUnit(
        index_, filename.c_str(), command_line_args.data(),
        command_line_args.size(), nullptr, 0, CXTranslationUnit_None);
    for (size_t i = 0; i < clang_getNumDiagnostics(unit_); ++i) {
      std::cout << "Warning encountered during parsing of translation unit:"
                << std::endl;
      std::cout << clang_getDiagnostic(unit_, i) << std::endl;
    }
    if (unit_ == nullptr) {
      throw std::runtime_error("Failed to parse the translation unit.");
    }
  }

  std::tuple<ASTNode *, Scope *> parse() {

    // Create root node and scope.
    CXCursor cursor = clang_getTranslationUnitCursor(unit_);

    AstFormatter formatter(2);
    clang_visitChildren(
        cursor, AstFormatter::traverse, &formatter);

    Scope *root_scope = new Scope("", nullptr);
    ASTNode *root_node =
        new ASTNode(cursor, ASTNodeType::ROOT, nullptr, root_scope);

    auto data = std::make_tuple(root_node, root_scope);
    clang_visitChildren(cursor, parse_clang_ast, &data);

    return std::make_tuple(root_node, root_scope);
  }

  ~Parser() {
    clang_disposeTranslationUnit(unit_);
    clang_disposeIndex(index_);
  }

private:
  CXIndex index_;
  CXTranslationUnit unit_;
};
} // namespace cxx
} // namespace pxx
#endif
