/** \file pxx/cxx/scope.h
 *
 * Defines the Scope class which keeps track of different names defines
 * in a C++ scope.
 */
#ifndef __PXX_CXX_SCOPE_H__
#define __PXX_CXX_SCOPE_H__

#include <map>
#include <string>
#include <vector>

namespace pxx {
namespace cxx {

class Type;

/** A scope that keeps track of declared names.
 *
 * The Scope class keeps track of different types, functions and classes
 * defined in it.
 */
class Scope {
 public:
  /** Create a new scope.
    * @param name The name of the scope.
    * @param parent_scope Pointer to parent scope or nullptr if root scope.
    */
  Scope(std::string name, Scope *parent_scope)
      : name_(name), parent_scope_(parent_scope) {
    }
  std::string get_name() { return name_; }

  /// Set the name of the scope.
  void set_name(std::string s) { name_ = s; }
  /// Add a name to the scope.
  void add_name(std::string name) { names_.push_back(name); }
  /// Set the parent of the scope.
  void set_parent(Scope *p) { parent_scope_ = p; }

  /** Determine how often a name has been defined.
   *
   * Counts how often a given name has been defined in this scope.
   * @param name The name to look up.
   * @return The number of definitions with the given name.
   */
  size_t get_n_definitions(std::string name) {
    size_t count = 0;
    auto end = names_.end();
    auto it = std::find(names_.begin(), end, name);
    while (it < end) {
      ++count;
      ++it;
      it = std::find(it, end, name);
    }
    return count;
  }

  /** Add type alias or type definition to the scope.
   *
   * @param name The alias.
   * @param value The type the alias refers to.
   */
  void add_type_alias(std::string name, std::string value) {
    auto it = std::find(type_names_.begin(), type_names_.end(), name);
    if (it == type_names_.end()) {
      type_names_.push_back(name);

      type_replacements_.push_back(value);
    } else {
      size_t pos = it - type_names_.begin();
      type_names_[pos] = name;
      type_replacements_[pos] = value;
    }
  }

  /** Register a user-defined type.
   *
   * Register a user-defined type using its Unified Symbol Resolution (USR).
   * This function registers a parsed type with the corresponding clang USR
   * in order to allow cross-referencing across the pxx AST.
   *
   * @param usr The USR of the clang cursor corresponding to the defined type.
   * @param type_ptr Pointer to type object that the USR 
   */
  void register_type(std::string usr, Type *type_ptr) {
    user_defined_types_.insert(std::make_pair(usr, type_ptr));
  }

  /** Update a type definition.
   * @param usr The USR of the type definition to update.
   * @param type_ptr Pointer to the type to update the current with.
   */
  void update_type(std::string usr, Type *type_ptr) {
    user_defined_types_[usr] = type_ptr;
  }

  /** Lookup type by USR.
   *
   * @param usr The USR of the type to lookup.
   * @return Pointer to the type or nullptr if the type was not defined in
   * this scope.
   */
  Type *lookup_type(std::string usr) {
    auto it = user_defined_types_.find(usr);
    if (it != user_defined_types_.end()) {
      return it->second;
    } else {
      if (parent_scope_) {
        return parent_scope_->lookup_type(usr);
      } else {
        return nullptr;
      }
    }
  }

  /** Return list of type replacements needed to make type name fully qualified.
   *
   * This retrieves a list of all type replacements that are required to turn
   * a type name used in a scope into a fully-qualified type name, which is
   * valid at root scope.
   *
   * @return A pair containing the vector of names and corresponding replacements.
   */
  std::pair<std::vector<std::string>, std::vector<std::string>>
  get_type_replacements() {
    std::vector<std::string> names{}, replacements{};
    if (parent_scope_) {
      std::tie(names, replacements) = parent_scope_->get_type_replacements();
    }
    names.insert(names.end(), type_names_.begin(), type_names_.end());
    replacements.insert(replacements.end(),
                        type_replacements_.begin(),
                        type_replacements_.end());
    return std::make_pair(names, replacements);
  }

  /** Get the type prefix of the current scope.
   * @return The prefix corresponding to the current scope.
   */
  std::string get_prefix() {
    std::string name = "";
    if (parent_scope_) {
      name += parent_scope_->get_prefix();
      name += name_;
    }
    if (name != "") {
      name += "::";
    }
    return name;
  }

  /** Join two scopes.
   *
   * This joins two separated scopes.
   *
   * @param other The other scope to join this scope with.
   */
  void join(Scope &other) {
    names_.insert(names_.end(), other.names_.begin(), other.names_.end());
    type_names_.insert(
        type_names_.end(), other.type_names_.begin(), other.type_names_.end());
    type_replacements_.insert(type_replacements_.end(),
                              other.type_replacements_.begin(),
                              other.type_replacements_.end());
    user_defined_types_.merge(other.user_defined_types_);
  }

 protected:
  std::string name_;
  Scope *parent_scope_;
  std::vector<std::string> names_ = {};
  std::vector<std::string> type_names_ = {};
  std::vector<std::string> type_replacements_ = {};
  std::map<std::string, Type *> user_defined_types_;
};

}  // namespace cxx
}  // namespace pxx

#endif
