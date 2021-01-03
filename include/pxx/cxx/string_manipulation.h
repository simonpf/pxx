/** \file pxx/cxx/string_manipulation.h
 *
 * Provides functions to replace names in strings.
 */
#ifndef __PXX_CXX_STRING_MANIPULATION_H__
#define __PXX_CXX_STIRNG_MANIPULATION_H__

#include <assert.h>
#include <regex>
#include <string>
#include <vector>
#include <iostream>

namespace pxx {
namespace cxx {
namespace detail {

static std::regex EXPRESSION_REGEX = std::regex("(^|[^:a-zA-Z_])([a-zA-Z_][a-zA-Z0-9_]*)");


/** Remove template arguments from type.
 *
 * @param s C++ type including a template type with given template argument.
 * @return The type with template arguments removed.
 */
inline std::string remove_template_arguments(std::string s) {
  size_t size = s.size();
  size_t last_colon = 0;
  size_t colon = s.find("::");
  while (colon < size) {
    last_colon = colon + 2;
    colon = s.find("::", last_colon);
  }
  size_t pos = s.find("<", last_colon);
  return std::string(s, 0, pos);
}

/** Replace type names.
 *
 * Textual replacement of type names.
 *
 * @param tname The type name containing template variables.
 * @param template_names The names of the template variables.
 * @param template_arguments The replacements for each variable.
 * @return The given typename with all template variables replaced
 *   with the corresponding types or variables.
 */
inline std::string replace_names(std::string name,
                                 const std::vector<std::string> &names,
                                 const std::vector<std::string> &values) {
  /// Number of names must be same size as values.
  assert(names.size() == values.size());

  // Search for valid C++ identifiers in name and replace.
  std::string input = name;
  std::smatch match;
  std::string result = name;

  bool any_match = true;

  while (any_match) {
    std::stringstream output{};
    std::regex_search(result, match, EXPRESSION_REGEX);
    any_match = false;

    while (!match.empty()) {
      std::string ms = match[2];

      // Copy what didn't match.
      output << match.prefix();
      output << match[1];

      bool matched = false;
      for (int i = 0; i < static_cast<int>(names.size()); ++i) {
        const std::string &n = names[i];

        if ((n == ms) && (ms != values[i])) {
          std::string s = match.suffix();
          std::string repl = values[i];
          if (n != repl.substr(0, n.size())) {
            if ((s.size() > 0) && (s[0] == '<')) {
              repl = remove_template_arguments(repl);
            }
            // Replace match.
            output << repl;
            matched = true;
            any_match = true;
            break;
          }
        }
      }
      // If didn't match, keep string.
      if (!matched) {
        output << ms;
      }
      input = match.suffix();
      std::regex_search(input, match, EXPRESSION_REGEX);
    }
    output << input;
    result = output.str();
  }
  return result;
}

namespace detail {
inline bool is_identifier_char(char c) { return (isalnum(c) || c == '_' || c == ':'); }

inline bool has_prefix(const std::string &name, size_t position) {
    return ((position > 0) && (is_identifier_char(name[position - 1])));
}

inline bool has_postfix(const std::string &name, size_t position) {
    return ((position < name.size()) && (is_identifier_char(name[position])));
}

inline bool is_full_match(const std::string &full_name,
                   const std::string &match,
                   size_t position) {
    return (!has_prefix(full_name, position)
            && !has_postfix(full_name, position + match.size()));
}


} // namespace detail

inline std::string
replace_unqualified_names(std::string name,
                          const std::vector<std::string> &names,
                          const std::vector<std::string> &values) {
  for (size_t i = 0; i < names.size(); ++i) {
    auto n = names[i];
    auto n_len = n.size();
    auto r = values[i];
    auto found = name.find(n);
    while (found != std::string::npos) {

      // Skip if previous char is part of identifier.
      if (detail::is_full_match(name, n, found)) {
        name = name.replace(found, n_len, r);
      }
      found = name.find(n, found + 1);

    }
  }
  return name;
}

}  // namespace detail
}  // namespace cxx
}  // namespace pxx

#endif
