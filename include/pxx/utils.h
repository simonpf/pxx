#ifndef __PXX_UTILS_H__
#define __PXX_UTILS_H__

#include <clang-c/Index.h>
#include <iostream>
#include <iomanip>
#include <string>

namespace pxx {

inline std::ostream &operator<<(std::ostream &stream, const CXString &s) {
  stream << clang_getCString(s);
  clang_disposeString(s);
  return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const CXCursor &c) {
  stream << clang_getCursorSpelling(c);
  return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const CXType &t) {
    stream << clang_getTypeSpelling(t);
    return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const CXCursorKind &k) {
  stream << clang_getCursorKindSpelling(k);
  return stream;
}

inline std::ostream &operator<<(std::ostream &stream, const CXDiagnostic &k) {
  stream << clang_getDiagnosticSpelling(k);
  return stream;
}

inline CXCursorKind kind(CXCursor c) { return clang_getCursorKind(c); }

inline void print_children(CXCursor c) {
  clang_visitChildren(
      c,
      [](CXCursor c, CXCursor /*parent*/, CXClientData /*client_data*/) {
        std::cout << "Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
                  << clang_getCursorKindSpelling(clang_getCursorKind(c))
                  << "'\n";
        return CXChildVisit_Continue;
      },
      nullptr);
}

inline std::string to_string(CXString clang_string) {
  auto sp = clang_getCString(clang_string);
  std::string s("");
  if (sp) {
    s = std::string(sp);
  }
  clang_disposeString(clang_string);
  return s;
}


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

}  // namespace pxx

#endif
