/** \file ast.h
 *
 * This file defines the AST class which parsed from the clang AST and
 * implements the required introspective functionality to generate the
 * Python interface.
 *
 * Author: Simon Pfreundschuh, 2020
 *
 */
#ifndef __PXX_AST_H__
#define __PXX_AST_H__

#include <clang-c/Index.h>
#include <pxx/cxx.h>
#include <pxx/utils.h>

#include <inja/inja.hpp>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace pxx {
namespace ast {

using pxx::operator<<;
using pxx::cxx::operator<<;
using pxx::comment_parser::ExportSettings;

namespace detail {

/** Formatted printing of Clang AST
 *
 * This class implements a printing class to dump the clang AST. Useful for
 * debugging.
 *
 */
struct AstFormatter {
  /** Create AST Formatter.
     * @param level Current indentation level.
     */
  AstFormatter(size_t level = 0) : level_(level) {}

  /** Print cursor.
     * @CXCursor Clang AST cursor representing the node to print.
     */
  void print(CXCursor c) {
    std::cout << std::setw(level_ * 4) << " "
              << " + ";
    std::cout << clang_getCursorKindSpelling(clang_getCursorKind(c));
    std::cout << " : " << clang_getCursorSpelling(c);

    CXCursorKind k = clang_getCursorKind(c);
    if (k == CXCursor_ClassDecl) {
      std::cout << "(Ref. template " << clang_getSpecializedCursorTemplate(c)
                << ")" << std::endl;
      ;
    }
    std::cout << std::endl;
  }

  /** Function to traverse Clang AST.
     *
     * This method provides the interface to the clang_visitChildren method.
     *
     */
  static CXChildVisitResult traverse(CXCursor c,
                                     CXCursor /*p*/,
                                     CXClientData d) {
    AstFormatter *formatter = reinterpret_cast<AstFormatter *>(d);
    formatter->print(c);
    formatter->level_++;
    clang_visitChildren(c, traverse, d);
    formatter->level_--;
    return CXChildVisit_Continue;
  }

  size_t level_ = 0;
};

}  // namespace detail

class AST {
 public:
  AST(CXCursor c, ExportSettings s = ExportSettings())
      : root_cursor_(c), root_scope_(cxx::parse<cxx::Namespace>(c, s)) {}
  friend std::ostream &operator<<(std::ostream &stream, const AST &tu);

  bool uses_standard_headers() { return root_scope_->uses_std(); }

  bool uses_eigen_headers() { return root_scope_->uses_eigen(); }

  void dump_ast() {
    detail::AstFormatter ast_formatter{};
    clang_visitChildren(
        root_cursor_, detail::AstFormatter::traverse, &ast_formatter);
  }

  void print() { std::cout << *this << std::endl; }

  friend std::ostream &operator<<(std::ostream &, const AST &ast);
  friend void to_json(json &j, const AST &ast);

 private:
  CXCursor root_cursor_;
  std::shared_ptr<cxx::Namespace> root_scope_;
};

std::ostream &operator<<(std::ostream &stream, const AST &ast) {
  stream << "::: C++ translation unit :::" << std::endl << std::endl;
  stream << "Defined functions:" << std::endl;
  for (auto &&f : ast.root_scope_->get_exported_functions()) {
    std::cout << std::endl;
    std::cout << *f;
  }
  stream << "Defined classes:" << std::endl;
  for (auto &&cl : ast.root_scope_->get_exported_classes()) {
    std::cout << std::endl;
    std::cout << *cl;
  }
  stream << "Defined namespaces:" << std::endl;
  for (auto &&n : ast.root_scope_->get_namespaces()) {
    std::cout << std::endl;
    std::cout << *n;
  }
  return stream;
}

void to_json(json &j, const AST &t) { to_json(j, t.root_scope_); }

}  // namespace ast
}  // namespace pxx

#endif
