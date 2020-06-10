#ifndef __PXX_PARSER_H__
#define __PXX_PARSER_H__

#include <clang-c/Index.h>
#include <string>
#include <iostream>
#include <pxx/ast.h>

namespace pxx {

class Parser {
public:
  Parser(std::string filename) {
    const char *command_line_args[] = {"-x", "c++"};
    index_ = clang_createIndex(0, 0);
    unit_ = clang_parseTranslationUnit(index_,
                                       filename.c_str(),
                                       command_line_args, 2,
                                       nullptr, 0,
                                       CXTranslationUnit_None);
    if (unit_ == nullptr) {
      throw std::runtime_error("Failed to parse the translation unit.");
    }
  }

  void print() {
    CXCursor cursor = clang_getTranslationUnitCursor(unit_);
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor /*parent*/, CXClientData /*client_data*/)
        {
            std::cout << "Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
                  << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "'\n";
            return CXChildVisit_Recurse;
        },
        nullptr);
  }

  ast::TranslationUnit parse() {
      CXCursor cursor = clang_getTranslationUnitCursor(unit_);
      ast::TranslationUnit tu{cursor};
      return tu;
  }

  ~Parser() {
    clang_disposeTranslationUnit(unit_);
    clang_disposeIndex(index_);
  }

private:
  CXIndex index_;
  CXTranslationUnit unit_;
};

} // namespace pxx

#endif
