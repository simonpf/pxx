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

CXCursorKind kind(CXCursor c) {
    return clang_getCursorKind(c);
}

} // namespace pxx

#endif
