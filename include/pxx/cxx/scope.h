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
#include "pxx/cxx/type_names.h"

namespace pxx {
namespace cxx {

class Scope;

namespace detail {
    std::string replace_names(const Scope *scope, std::string name);
}  // namespace detail

class Type;

/** Scopes of names.
 *
 * The Scope class keeps track of definitions of types and their names, so that
 * names from a given scope can be mapped to the root scope.
 *
 * All scopes except for the root scope have a parent scope. Each scope keeps track
 * of the names defined in it as well as child scopes.
 *
 */
class Scope {
 public:

   /** Create a new scope.
    *
    * Creates a new scope with the given name and registers the scope as a child
    * scope of the parent.
    *
    * @param name The name of the scope.
    * @param parent_scope Pointer to parent scope or nullptr if root scope.
    */
   Scope(std::string name, Scope *parent_scope)
       : name_(name), parent_scope_(parent_scope) {
     if (parent_scope_) {
       parent_scope_->add_child(name_, this);
     }
   }

   /// The name of the scope.
   std::string get_name() const { return name_; }

   /// Set the name of the scope.
   void set_name(std::string s) {
     if (parent_scope_) {
       parent_scope_->remove_child(name_);
       parent_scope_->add_child(s, this);
     }
     name_ = s;
  }

  /// Add a name to the scope.
  void add_name(std::string name) { names_.push_back(name); }

  void replace_name(std::string old_name, std::string new_name) {
      auto found = std::find(names_.begin(), names_.end(), old_name);
      if (found != names_.end()) {
          *found = new_name;
      } else {
          names_.push_back(new_name);
      }
  }

  /// Set the parent of the scope.
  void set_parent(Scope *p) {
      if (parent_scope_) {
          parent_scope_->remove_child(name_);
      }
      p->add_child(name_, this);
      parent_scope_ = p;
  }

  /** Return pointer to child scope.
   *
   * @param name Name of the child scope.
   * @return Pointer to child scope or nullptr if no scope with the given name
   * is present.
   */
  Scope *get_child(const std::string &name) {
      auto found = child_scopes_.find(name);
      if (found == child_scopes_.end()) {
          return nullptr;
      }
      return found->second;
  }

  /// Register child scope.
  void add_child(std::string name, Scope *p) {
      child_scopes_.insert(std::make_pair(name, p));
  }
  /// Remove child scope.
  void remove_child(std::string name) {
      child_scopes_.erase(name);
  }

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
  std::string get_prefix() const {
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

    for (auto &s : other.child_scopes_) {
        auto found = child_scopes_.find(s.first);
        if (found != child_scopes_.end()) {
            child_scopes_[s.first]->join(*s.second);
        } else {
            child_scopes_.insert(s);
        }
    }
    //child_scopes_.merge(other.child_scopes_);
  }

  std::string lookup_name(const std::string &name) const {
    // Qualified lookup.
    if (type_names::is_qualified(name)) {
      auto prefix = type_names::get_prefix(name);
      auto found = child_scopes_.find(prefix);
      if (found != child_scopes_.end()) {
        return found->second->lookup_name(type_names::get_suffix(name));
      }
    }
    // Unqualified lookup.
    auto found = std::find(names_.begin(), names_.end(), name);
    if (found != names_.end()) {
      return get_prefix() + name;

    }
    // Unqualified lookup with replacement.
    found = std::find(type_names_.begin(), type_names_.end(), name);
    if (found != type_names_.end()) {
      size_t index = found - type_names_.begin();
      return detail::replace_names(this, type_replacements_[index]);
    }

    if (parent_scope_) {
      return parent_scope_->lookup_name(name);
    }
    return "";
  }

  /** Resolve type names in type.
   *
   * Iterates over identifiers in the given type name and replaces them with the
   * qualified names that identify them at root scope.
   *
   * @param name String containing a C++ type name.
   * @return The name with all unqualified names replaced with their qualified
   * counterparts.
   */
  std::string get_qualified_name(std::string name) const {
    return detail::replace_names(this, name);
  }

 protected:
  std::string name_;
  Scope *parent_scope_;
  std::vector<std::string> names_ = {};
  std::vector<std::string> type_names_ = {};
  std::vector<std::string> type_replacements_ = {};
  std::map<std::string, Type *> user_defined_types_ = {};
  std::map<std::string, Scope *> child_scopes_ = {};
};

namespace detail {

    std::string replace_names(const Scope *scope, std::string name) {
        std::regex_iterator<std::string::iterator> it(
            name.begin(), name.end(), type_names::IDENTIFIER);
        std::regex_iterator<std::string::iterator> end;

        std::string output = name;

        int output_offset = 0;

        while (it != end) {
            auto match = it->str();
            auto repl = scope->lookup_name(match);
            if ((repl != "") && (repl != match)) {
                output.replace(it->position() + output_offset, match.size(), repl);
                output_offset += repl.size() - match.size();
            }
            ++it;
        }
        return output;
    }

}  // namespace detail

}  // namespace cxx
}  // namespace pxx

#endif
