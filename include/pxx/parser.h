#ifndef __PXX_PARSER_H__
#define __PXX_PARSER_H__

#include <clang-c/Index.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <pxx/ast.h>

namespace pxx {

class Parser {
public:
  Parser(std::string filename,
         std::vector<std::string> additional_args = {}) {
    std::vector<const char *> command_line_args = {"-x",
                                                   "c++",
                                                   "-std=c++11",
                                                   "-fparse-all-comments"};
    for (auto & s : additional_args) {
      command_line_args.push_back(s.c_str());
    }
    index_ = clang_createIndex(0, 0);
    unit_ =
        clang_parseTranslationUnit(index_, filename.c_str(), command_line_args.data(),
                                   command_line_args.size(), nullptr, 0, CXTranslationUnit_None);
    for (size_t i = 0; i < clang_getNumDiagnostics(unit_); ++i) {
        std::cout << "Warning encountered during parsing of translation unit:" << std::endl;
        std::cout << clang_getDiagnostic(unit_, i) << std::endl;
    }
    if (unit_ == nullptr) {
      throw std::runtime_error("Failed to parse the translation unit.");
    }
  }

  ast::AST parse() {
    CXCursor cursor = clang_getTranslationUnitCursor(unit_);
    ast::AST tu{cursor, settings_};
    return tu;
  }

  void set_export_default() {
      settings_.exp = true;
  }

  void set_hide_default() {
      settings_.exp = false;
  }

  ~Parser() {
    clang_disposeTranslationUnit(unit_);
    clang_disposeIndex(index_);
  }

private:
  CXIndex index_;
  CXTranslationUnit unit_;
  ast::ExportSettings settings_;
};

} // namespace pxx

#endif
