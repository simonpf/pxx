/** ifile pxx/cxx/common.h
 *
 * This file contains general definition that are required in multiple
 * places.
 */
#ifndef __PXX_CXX_COMMON_H__
#define __PXX_CXX_COMMON_H__

#include <unordered_set>
#include <string>

namespace pxx {
namespace cxx {
static std::unordered_set<std::string> keywords = {"alignas",
                                                   "alignof",
                                                   "and",
                                                   "and_eq",
                                                   "asm",
                                                   "atomic_cancel",
                                                   "atomic_commit",
                                                   "atomic_noexcept",
                                                   "auto",
                                                   "bitand",
                                                   "bitor",
                                                   "bool",
                                                   "break",
                                                   "case",
                                                   "catch",
                                                   "char",
                                                   "char8_t",
                                                   "char16_t",
                                                   "char32_t",
                                                   "class",
                                                   "compl",
                                                   "concept",
                                                   "const",
                                                   "consteval",
                                                   "constexpr",
                                                   "constinit",
                                                   "const_cast",
                                                   "continue",
                                                   "co_await",
                                                   "co_return",
                                                   "co_yield"
                                                   "decltype",
                                                   "default",
                                                   "delete",
                                                   "do",
                                                   "double",
                                                   "dynamic_cast",
                                                   "else",
                                                   "enum",
                                                   "explicit",
                                                   "export",
                                                   "extern",
                                                   "false",
                                                   "float",
                                                   "for",
                                                   "friend",
                                                   "goto",
                                                   "if",
                                                   "inline",
                                                   "int",
                                                   "long",
                                                   "mutable",
                                                   "namespace",
                                                   "new",
                                                   "noexcept",
                                                   "not",
                                                   "not_eq",
                                                   "nullptr",
                                                   "operator",
                                                   "or",
                                                   "or_eq",
                                                   "private",
                                                   "protected",
                                                   "public",
                                                   "reflexpr",
                                                   "register",
                                                   "reinterpret_cast",
                                                   "requires",
                                                   "return",
                                                   "short",
                                                   "signed",
                                                   "sizeof",
                                                   "static",
                                                   "static_assert",
                                                   "static_cast",
                                                   "struct",
                                                   "switch",
                                                   "synchronized",
                                                   "template",
                                                   "this",
                                                   "thread_local",
                                                   "throw",
                                                   "true",
                                                   "try",
                                                   "typedef",
                                                   "typeid",
                                                   "typename",
                                                   "union",
                                                   "unsigned",
                                                   "using",
                                                   "virtual",
                                                   "void",
                                                   "volatile",
                                                   "wchar_t",
                                                   "while",
                                                   "xor",
                                                   "xor_eq"};

inline bool
is_keyword(std::string identifier) {
  auto found = keywords.find(identifier);
  return found != keywords.end();
}

} // namespace cxx
} // namespace pxx

#endif
