/** \file pxx/cxx/type_names.h
 *
 * Functions to manipulate type names.
 *
 */
#include <string>
#include <regex>

namespace pxx {
namespace cxx {
namespace type_names {
/// Matches identifiers in type names.
static std::regex IDENTIFIER = std::regex("([\\w:_]+)");

inline bool is_qualified(const std::string &name) {
  return name.find("::") != std::string::npos;
}

inline std::string get_prefix(const std::string &name) {
  auto found = name.find("::");
  return std::string(name, 0, found);
}

inline std::string get_suffix(std::string name) {
  auto found = name.find("::");
  return std::string(name, found + 2);
}

} // namespace type_names
} // namespace cxx
} // namespace pxx
