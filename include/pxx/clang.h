/** \file clang.h
 *
 * This file contains the pxx::clang namespace which defines helper functions
 * for the handling of libclang object.
 *
 * Author: Simon Pfreundschuh, 2021
 *
 */
#ifndef __PXX_CLANG_H__
#define __PXX_CLANG_H__

#include <clang-c/Index.h>
#include <string>

namespace pxx {
namespace clang {

/** Return cursor name as string.
 *
 * @param cursor The libclang CXCursor for which to return the name.
 * @return The cursor name as string.
 */
inline std::string get_cursor_name(CXCursor cursor) {
  CXString s = clang_getCursorSpelling(cursor);
  return pxx::to_string(s);
}

/** Return spelling of type.
 *
 * @param type The libclang CXType to return the spelling for.
 * @return The spelling as std::string.
 */
inline std::string get_type_spelling(CXType type) {
  CXString s = clang_getTypeSpelling(type);
  return pxx::to_string(s);
}

/** Return source location of cursor.
 *
 * @param cursor The libclang cursor for which to return the source location.
 * @return Tuple containing the path of the source file as well as the row
 * and column numbers.
 */
inline std::tuple<std::filesystem::path, size_t, size_t>
    get_cursor_location(CXCursor cursor) {
  CXFile file;
  unsigned line_number, column, offset;
  auto location = clang_getCursorLocation(cursor);
  clang_getSpellingLocation(location, &file, &line_number, &column, &offset);
  std::string file_name = pxx::to_string(clang_getFileName(file));
  return std::make_tuple(file_name, line_number, column);
}

/** Return the qualified name of a cursor.
 *
 * @param cursor The cursor for which to return the qualified name.
 * @return The qualified name of the given cursor as std::string.
 */
inline std::string get_qualified_name(CXCursor cursor) {

  auto parent = cursor;
  std::vector<std::string> names = {};
  while (!clang_isTranslationUnit(clang_getCursorKind(parent))) {
    names.push_back(get_cursor_name(parent));
    parent = clang_getCursorSemanticParent(parent);
    std::cout << "parent: " << parent << std::endl;
  }

  std::stringstream stream = std::stringstream{};
  size_t n_names = names.size();
  for (size_t i = n_names; i > 0; --i) {
    stream << names[i - 1];
    if (i > 1) {
      stream << "::";
    }
  }
  return stream.str();
}
} // namespace clang
} // namespace pxx

#endif
