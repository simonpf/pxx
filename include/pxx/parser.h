#ifndef __PXX_PARSER_H__
#define __PXX_PARSER_H__

#include <clang-c/Index.h>
#include <string>
#include <iostream>

namespace pxx {

std::ostream &operator<<(std::ostream &stream, const CXString &s) {
  stream << clang_getCString(s);
  clang_disposeString(s);
  return stream;
}

class Parser {
public:
  Parser(std::string filename) {

    index_ = clang_createIndex(0, 0);
    unit_ = clang_parseTranslationUnit(index_, filename.c_str(), nullptr, 0, nullptr, 0,
                                       CXTranslationUnit_None);
    if (unit_ == nullptr) {
      throw std::runtime_error("Failed to parse the translation unit.");
    }
  }

  void print() {
    CXCursor cursor = clang_getTranslationUnitCursor(unit_);
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            std::cout << "Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
                  << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "'\n";
            return CXChildVisit_Recurse;
        },
        nullptr);
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
