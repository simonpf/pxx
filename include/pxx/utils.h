#ifndef __PXX_UTILS_H__
#define __PXX_UTILS_H__

#include <clang-c/Index.h>
#include <iostream>
#include <string>

namespace pxx {

std::ostream &operator<<(std::ostream &stream, const CXString &s) {
  stream << clang_getCString(s);
  clang_disposeString(s);
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const CXCursor &c) {
  stream << clang_getCursorSpelling(c);
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const CXCursorKind &k) {
    stream << clang_getCursorKindSpelling(k);
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const CXDiagnostic &k) {
    stream << clang_getDiagnosticSpelling(k);
    return stream;
}

CXCursorKind kind(CXCursor c) {
    return clang_getCursorKind(c);
}

void print_children(CXCursor c) {
    clang_visitChildren(c,
                        [](CXCursor c, CXCursor /*parent*/, CXClientData /*client_data*/)
                        {
                            std::cout << "Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
                                      << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "'\n";
                            return CXChildVisit_Continue;
                        },
                        nullptr);
}

std::string to_string(CXString clang_string) {
  auto sp = clang_getCString(clang_string);
  std::string s("");
  if (sp) {
    s = std::string(sp);
  }
  clang_disposeString(clang_string);
  return s;
}

} // namespace pxx

#endif
