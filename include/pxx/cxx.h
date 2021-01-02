/** \file cxx.h
 *
 * This file contains classes representing the C++ language constructs that
 * are required to implement the pxx functionality.
 *
 * Author: Simon Pfreundschuh, 2020
 *
 */
#ifndef __PXX_CXX_H__
#define __PXX_CXX_H__

#include <pxx/cxx/string_manipulation.h>
#include <pxx/cxx/scope.h>
#include <pxx/cxx/language_object.h>
#include <pxx/utils.h>

#include <inja/inja.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace pxx {
namespace cxx {

using pxx::operator<<;
using json = nlohmann::json;

namespace detail {

using pxx::to_string;

enum class access_specifier_t { pub, prot, priv };
std::string to_string(access_specifier_t as) {
  switch (as) {
    case access_specifier_t::pub:
      return "public";
    case access_specifier_t::prot:
      return "protected";
    case access_specifier_t::priv:
      return "private";
  }
  return "";
}

enum class storage_class_t { ext, stat, none };
std::string to_string(storage_class_t sc) {
  switch (sc) {
    case storage_class_t::ext:
      return "extern";
    case storage_class_t::stat:
      return "static";
    case storage_class_t::none:
      return "none";
  }
  return "";
}

std::string get_name(CXCursor c) {
  CXString s = clang_getCursorSpelling(c);
  return pxx::to_string(s);
}

template <typename T, typename S>
void set_parents(std::vector<std::shared_ptr<T>> &ts, S *ptr) {
  for (auto &t : ts) {
    t->set_parent(ptr);
  }
}

std::string remove_scope_prefix(std::string prefix, std::string name) {
  if (prefix == "") {
    return name;
  }

  std::string result = name;
  size_t l = result.size();
  size_t p = result.find(prefix);
  while (p < l) {
    result.replace(p, prefix.size(), "");
    l = result.size();
    p = result.find(prefix);
  }
  return result;
}

}  // namespace detail

template <typename T>
void to_json(json &j, std::shared_ptr<T> pt) {
  if (pt) {
    to_json(j, *pt);
  }
}
class Type;

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

/** A C++ type.
 * Represents types associated with C++ language objects.
 */
class Type {
 public:
  /** Create type object from libclang type.
     * @param t CXType to represent.
     * @param p Scope  which the type should be interpreted.
     */
  Type(CXType t, Scope *p) : type_(t), scope_(p) {
    name_ = to_string(clang_getTypeSpelling(type_));
    name_ = detail::remove_scope_prefix(scope_->get_prefix(), name_);

    is_pointer_ = (type_.kind == CXType_Pointer);
    is_lvalue_reference_ = (type_.kind == CXType_LValueReference);
    is_rvalue_reference_ = (type_.kind == CXType_RValueReference);
    is_const_ = clang_isConstQualifiedType(type_);
  }

  /** Create type object from libclang cursor.
   * @param c Cursor pointing to typed object.
   * @param p Scope  which the type should be interpreted.
   */
  Type(CXCursor c, Scope *p) : Type(clang_getCursorType(c), p) {}

  /** Clone type object.
     *
     * Creates an independent clone of the type object on the heap and
     * updates the pointer to the parent scope according to the provided
     * parent.
     *
     * @p Pointer to cloned parent.
     * @return Smart pointer to clone object.
     */
  std::shared_ptr<Type> clone(LanguageObject *p) {
    auto t_new = std::make_shared<Type>(*this);
    t_new->set_scope(p->get_parent()->get_scope());
    return t_new;
  }

  /// The type's name.
  std::string get_name() const {
    // libclang returns inconsistent type names in class template specializations.
    // This deals with this issue.
    if (name_.find('-') < name_.size()) {
      return get_unqualified_name();
    }
    return name_;
  }

  /// The type's name that identifies it at root scope.
  std::string get_qualified_name() const {
    std::vector<std::string> names, values;
    std::tie(names, values) = scope_->get_type_replacements();

    std::string qualified_name =
        detail::replace_names(get_name(), names, values);
    return qualified_name;
  }

  std::string get_unqualified_name() const {
    std::regex re("[-a-zA-Z0-9_][-a-zA-Z0-9, :<>_]*::");
    return std::regex_replace(name_, re, "");
    size_t p = name_.find_last_of(':');
    return std::string(name_, p + 1, name_.size() - p - 1);
    //std::regex re("([a-zA-Z_][a-zA-Z0-9_]*::)*([a-zA-Z_][a-zA-Z0-9_]*)");
    std::smatch match;
    bool found = regex_match(name_, match, re);
    if (found) {
      return match[2];
    }
    return name_;
  }
  /** Replace template variables in type names.
   *
   * Replace template variables in the types name and canonical name.
   *
   * @param template_names Vector containing the names of the template variables to replace.
   * @param template_arguments Vector containing the types and variables with which to
   *    replace the template variables.
   */
  void replace_template_variables(const std::vector<std::string> &names,
                                  const std::vector<std::string> &values) {
    name_ = detail::replace_names(name_, names, values);
  }

  /// Is the type const qualified?
  bool is_const() const { return is_const_; }
  /// Is the type a pointer?
  bool is_pointer() const { return is_pointer_; }
  /// Is the type a l-value reference?
  bool is_lvalue_reference() const { return is_lvalue_reference_; }
  /// Is the type a r-value reference?
  bool is_rvalue_reference() const { return is_rvalue_reference_; }
  /// Set scope in which the type is defined.
  void set_scope(Scope *s) { scope_ = s; }

  friend std::ostream &operator<<(std::ostream &stream, const Type &t);
  friend bool operator==(const Type &, const Type &);

 protected:
  std::string name_;
  std::string usr_;
  CXType type_;
  bool is_const_;
  bool is_pointer_;
  bool is_lvalue_reference_;
  bool is_rvalue_reference_;
  Scope *scope_;
};

bool operator==(const Type &l, const Type &r) {
  return clang_equalTypes(l.type_, r.type_);
}

/** JSON serialization of Type
 *
 * @param j JSON handle serving as destination.
 * @param lo The language object to serialize.
 */
void to_json(json &j, const Type &t) {
  j["name"] = t.get_name();
  j["qualified_name"] = t.get_qualified_name();
}

std::ostream &operator<<(std::ostream &stream, const Type &t) {
  stream << t.get_name() << " (" << t.get_qualified_name() << ") ";
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/** A C++ variable.
 * Represents C++ variables appearing as function arguments or data members
 * of classes.
 */
class Variable : public LanguageObject {
 public:
  /** Create variable object from libclang cursor.
   * @param c libclang cursor representing a CXX_ParmDecl or similar.
   * @param p Pointer to parent object.
   */
  Variable(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p), type_(c, p->get_scope()) {}

  Type get_type() { return type_; }
  /// Return name (spelling) of the variable's type.
  std::string get_typename() const { return type_.get_name(); }

  /// Is the variable const qualified?
  bool is_const() const { return type_.is_const(); }

  /** Clone variable.
   *
   * Creates clone of this variable object and it descendants on heap
   * and updates its parent pointer.
   *
   * @param p The new parent.
   * @return Smart pointer to newly created clone.
   */
  std::shared_ptr<Variable> clone(LanguageObject *p) {
    std::shared_ptr<Variable> v = pxx::cxx::clone(*this, p);
    v->set_parent(p);
    v->type_ = *v->type_.clone(v.get());
    return v;
  }

  /** Replace template variables with values.
   *
   * Recurrent replacement of template variable with specific types or values.
   * Note that this changes the AST and should therefore be performed only
   * on independent copies of the original AST.
   *
   * @param template_names Names of the template arguments to replace.
   * @param template_values Values with which to replace the template arguments.
   */
  void replace_template_variables(
      const std::vector<std::string> &template_names,
      const std::vector<std::string> &template_values) {
    type_.replace_template_variables(template_names, template_values);
  }

  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const Variable &v);

 protected:
  std::string typename_;
  Type type_;
};

/** JSON serialization of Type
 *
 * @param j JSON handle serving as destination.
 * @param lo The language object to serialize.
 */
void to_json(json &j, const Variable &v) {
  to_json(j, static_cast<LanguageObject>(v));
  j["type"] = v.type_;
  j["const"] = v.type_.is_const();
}

std::ostream &operator<<(std::ostream &stream, const Variable &v) {
  stream << v.typename_ << " " << v.name_ << std::endl;
  return stream;
}

/** Data member of a class.
 *
 * Specializes Variable class to represent member variables of
 * classes.
 *
 */
class DataMember : public Variable {
 public:
  /** Create DataMember from libclang cursor.
     * @param c libclang cursor representing of kind CXCursor_FieldDecl
     * or similar.
     */
  DataMember(CXCursor c, LanguageObject *p) : Variable(c, p) {
    // retrieve access specifier.
    CX_CXXAccessSpecifier as = clang_getCXXAccessSpecifier(c);
    if (as == CX_CXXPublic) {
      access_specifier_ = detail::access_specifier_t::pub;
    } else if (as == CX_CXXProtected) {
      access_specifier_ = detail::access_specifier_t::prot;
    } else if (as == CX_CXXPrivate) {
      access_specifier_ = detail::access_specifier_t::priv;
    } else {
      valid_ = false;
    }

    // retrieve storage class.
    CX_StorageClass sc = clang_Cursor_getStorageClass(c);
    if (sc == CX_SC_None) {
      storage_class_ = detail::storage_class_t::none;
    } else if (sc == CX_SC_Extern) {
      storage_class_ = detail::storage_class_t::ext;
    } else if (sc == CX_SC_Static) {
      storage_class_ = detail::storage_class_t::stat;
    } else {
      valid_ = false;
    }
  }

  /** Clone member variable.
   *
   * @param p The new parent.
   * @return Smart pointer to newly created clone.
   */
  std::shared_ptr<DataMember> clone(LanguageObject *p) {
    std::shared_ptr<DataMember> v = std::make_shared<DataMember>(*this);
    v->set_parent(p);
    v->type_ = *v->type_.clone(v.get());
    return v;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const DataMember &v);

 private:
  detail::access_specifier_t access_specifier_;
  detail::storage_class_t storage_class_;
  bool valid_ = true;
};

/** JSON serialization of DataMember
 *
 * @param j JSON handle serving as destination.
 * @param lo The language object to serialize.
 */
void to_json(json &j, const DataMember &v) {
  to_json(j, static_cast<Variable>(v));
  j["access"] = to_string(v.access_specifier_);
  j["storage"] = to_string(v.storage_class_);
}

////////////////////////////////////////////////////////////////////////////////
// C++ Templates
////////////////////////////////////////////////////////////////////////////////

/** Generic template class.
 *
 * This class represents template of classes, function and type aliases.
 *
 * @tparam Base C++ AST type for which this class is a template.
 */
template <typename Base>
class Template {
 public:
  /** Create template.
     *
     * @t The definition of the template.
     */
  Template(std::shared_ptr<Base> t) : template_(t) {}

  virtual ~Template() = default;

  ///  Return pointer to AST node containing the template definition.
  Base *get_base() { return template_.get(); }

  /** Add template argument.
   *
   * This function is during parsing to add arguments to the template.
   *
   * @lo LanguageObject representing the template argument to add.
   */
  void add(std::shared_ptr<LanguageObject> lo) { arguments_.push_back(lo); }

  std::vector<std::string> get_template_arguments() const {
    std::vector<std::string> template_arguments;
    for (auto &ta : arguments_) {
      template_arguments.push_back(ta->get_name());
    }
    return template_arguments;
  }

  /** Return name of template instance.
   *
   * Adds the given template values in angle brackets to the template name.
   *
   * @param template_replacements Template values to insert for each template argument
   * @return The name (spelling) of the instance of the template.
   */
  virtual std::string get_template_name(
      std::vector<std::string> template_replacements) const {
    std::stringstream ss;
    ss << template_->get_name() << "<";
    for (size_t i = 0; i < template_replacements.size(); ++i) {
      ss << template_replacements[i];
      if (i < template_replacements.size() - 1) {
        ss << ",";
      }
    }
    ss << ">";
    return ss.str();
  }

  /** Get template instances to export.
   *
   * @return Vector containing the template instances corresponding to
   * the instances of this template.
   */
  std::vector<std::shared_ptr<Base>> get_instances() const {
    auto template_arguments = get_template_arguments();
    std::vector<std::shared_ptr<Base>> instances{};
    auto settings = template_->get_export_settings();
    for (auto &is : settings.instance_strings) {
      std::string export_name = std::get<0>(is);
      if (export_name == "") {
        export_name = template_->get_name();
      }
      auto template_instance = template_->clone();
      template_instance->set_export_name(export_name);
      template_instance->set_name(get_template_name(std::get<1>(is)));
      template_instance->replace_template_variables(template_arguments,
                                                    std::get<1>(is));
      instances.push_back(template_instance);
    }
    return instances;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Template &f) {
    stream << "template<";
    for (size_t i = 0; i < f.arguments_.size(); ++i) {
      auto &p = f.arguments_[i];
      stream << p->get_name();
      if (i < f.arguments_.size() - 1) {
        stream << ", ";
      }
    }
    stream << "> " << static_cast<Base>(f);
    return stream;
  }

  void set_parent(LanguageObject *parent) { template_->set_parent(parent); }

 protected:
  std::shared_ptr<Base> template_ = nullptr;
  std::vector<std::shared_ptr<LanguageObject>> arguments_ = {};
};

template <typename Base>
class TemplateSpecialization : public Template<Base> {
 public:
  using Template<Base>::get_template_arguments;
  /** Create template.
     *
     * @t The definition of the template.
     */
  TemplateSpecialization(CXCursor c, std::shared_ptr<Base> t)
      : Template<Base>(t) {
    template_name_ = to_string(clang_getCursorDisplayName(c));
    template_name_ = std::string(template_name_, 0, template_name_.find('('));
  }

  std::string get_template_name(
      std::vector<std::string> template_replacements) const {
    auto template_arguments = get_template_arguments();
    std::string template_name = template_name_;
    template_name = detail::replace_names(
        template_name_, template_arguments, template_replacements);

    std::vector<std::string> names, values;
    std::tie(names, values) = template_->get_scope()->get_type_replacements();
    size_t p = std::find(names.begin(), names.end(), template_->get_name()) -
               names.begin();
    names.erase(names.begin() + p, names.begin() + p + 1);
    values.erase(values.begin() + p, values.begin() + p + 1);
    template_name = detail::replace_names(template_name, names, values);

    return template_name;
  }

 protected:
  using Template<Base>::template_;
  std::string template_name_;
};

////////////////////////////////////////////////////////////////////////////////
// Type aliases
////////////////////////////////////////////////////////////////////////////////

/** Type alias
 *
 * A type alias created using 'using' or 'typedef' expression. Keeping
 * track of those is required to determine the right spelling of types
 * for the interface.
 */
class TypeAlias : public LanguageObject {
 public:
  TypeAlias(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p),
        usr_(to_string(clang_getCursorUSR(c))),
        type_(c, p->get_scope()),
        underlying_type_(clang_getTypedefDeclUnderlyingType(c),
                         p->get_scope()) {
    p->get_scope()->register_type(to_string(clang_getCursorUSR(c)), &type_);
    p->get_scope()->add_type_alias(type_.get_unqualified_name(),
                                   underlying_type_.get_qualified_name());
  }

  void update_reference(Type *type_ptr) {
    if ((type_ptr) && (underlying_type_ == *type_ptr)) {
      underlying_type_ = *type_ptr;
      parent_->get_scope()->add_type_alias(
          name_, underlying_type_.get_qualified_name());
    }
  }

  std::shared_ptr<TypeAlias> clone(LanguageObject *p) {
    auto ta_new = pxx::cxx::clone(*this, p);
    ta_new->parent_->get_scope()->update_type(usr_, &ta_new->type_);
    ta_new->type_.set_scope(parent_->get_scope());
    return ta_new;
  }

  void replace_template_variables(const std::vector<std::string> &names,
                                  const std::vector<std::string> &values) {
    std::string qualified_name = detail::replace_names(
        underlying_type_.get_qualified_name(), names, values);
    parent_->get_scope()->add_type_alias(type_.get_unqualified_name(),
                                        qualified_name);
  }

 protected:
  std::string usr_;
  Type type_;
  Type underlying_type_;
};

/** Template alias
 *
 * A template type alias created using a 'using'.
 */
class TemplateAlias : public LanguageObject {
 public:
  TemplateAlias(CXCursor c, LanguageObject *p) : LanguageObject(c, p) {
    p->get_scope()->add_type_alias(name_, get_qualified_name());
  }

  std::shared_ptr<TemplateAlias> clone(LanguageObject *p) {
    auto ta_new = pxx::cxx::clone(*this, p);
    return ta_new;
  }

  void replace_template_variables(const std::vector<std::string> &names,
                                  const std::vector<std::string> &values) {
    std::string qualified_name =
        detail::replace_names(get_qualified_name(), names, values);
    parent_->get_scope()->add_type_alias(name_, qualified_name);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Enum
////////////////////////////////////////////////////////////////////////////////

/** An Enum declaration
 *
 * Enums are named entities so we have to keep track of them in order
 * to get their names right.
 */
class Enum : public LanguageObject {
 public:
  Enum(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p), scope_(name_, p ? p->get_scope() : nullptr) {
    p->get_scope()->add_type_alias(get_name(), get_qualified_name());
  }

  void add(std::shared_ptr<LanguageObject> c) { constants_.push_back(c); }

  void set_parent(LanguageObject *p) {
    scope_.set_parent(p->get_scope());
    this->LanguageObject::set_parent(p);
    for (auto &c : constants_) {
      c->set_parent(this);
    }
  }

  virtual Scope *get_scope() {
    if (scope_.get_name() != "") {
      return &scope_;
    } else {
      return parent_scope_;
    }
  }

  friend void to_json(json &j, const Enum &e);

 private:
  Scope scope_;
  std::vector<std::shared_ptr<LanguageObject>> constants_ = {};
};

void to_json(json &j, const Enum &e) {
  to_json(j, reinterpret_cast<const LanguageObject &>(e));
  j["constants"] = e.constants_;
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

/** A C++ Function.
 *
 * Represents independent functions defined at namespace scope.
 */
class Function : public LanguageObject {
 public:
  /** Create function from cursor.
     *
     * @param c libclang cursor of kind CXCursor_FunctionDecl or similar.
     * @param p Pointer to parent object or nullptr for root of AST
     */
  Function(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p),
        return_type_(clang_getResultType(clang_getCursorType(c)),
                     p->get_scope()) {
    p->get_scope()->add_name(name_);
  }

  /** Add argument to function
   * This is used during the parsing of the function definition to register
   * its arguments.
   * @param Smart pointer to variable object representing a function argument.
   */
  void add(std::shared_ptr<Variable> a) { arguments_.push_back(a); }

  /** Replace occurrences of template names in function argument types.
   *
   * @param template_names Names of the template variable to replace.
   * @param template_values The values to replace the template variables with.
   *
   */
  void replace_template_variables(
      const std::vector<std::string> &template_names,
      const std::vector<std::string> &template_values) {
    for (auto &p : arguments_) {
      p->replace_template_variables(template_names, template_values);
    }
    return_type_.replace_template_variables(template_names, template_values);
  }

  /** Clone function object.
   *
   * Creates a clone of the function object on the heap and updates
   * its parent pointer.
   *
   * @param p The new parent of the clone object.
   * @return Smart pointer to cloned  function object.
   */
  std::shared_ptr<Function> clone(LanguageObject *p = nullptr) {
    auto f = pxx::cxx::clone<Function>(*this, p);
    std::transform(f->arguments_.begin(),
                   f->arguments_.end(),
                   f->arguments_.begin(),
                   [&f](auto a) { return a->clone(f.get()); });
    f->return_type_ = *f->return_type_.clone(f.get());
    return f;
  }

  /** Return spelling of corresponding function pointer type.
   *
   * Prints the function type, which is required to identify the function
   * from potential overloads.
   *
   * @return The spelling of a pointer to the function.
   */
  std::string get_pointer_type_spelling() const {
    std::stringstream ss;
    ss << "(" << return_type_.get_name() << ")(*)(";
    for (size_t i = 0; i < arguments_.size(); ++i)
      ({
        ss << (arguments_)[i]->get_type().get_name();
        if (i < arguments_.size() - 1) {
          ss << ", ";
        }
      });
    ss << ")";
    return ss.str();
  }

  /// True if there exists overloads with the scope in which the function is defined.
  bool has_overload() const {
    return (parent_->get_scope()->get_n_definitions(name_) > 1);
  }

  friend std::ostream &operator<<(std::ostream &stream, const Function &cl);
  friend void to_json(json &j, const Function &f);

 protected:
  std::vector<std::shared_ptr<Variable>> arguments_;
  Type return_type_;
};

using FunctionTemplate = Template<Function>;

/** JSON serialization of Function
 *
 * @param j JSON handle serving as destination.
 * @param lo The language object to serialize.
 */
void to_json(json &j, const Function &f) {
  to_json(j, static_cast<LanguageObject>(f));
  j["arguments"] = f.arguments_;
  j["return_type"] = f.return_type_;
  j["has_overload"] = f.has_overload();
  j["pointer_type"] = f.get_pointer_type_spelling();
}

/** Class member function.
 *
 * Specialization of Function class to take into account the
 * different spelling of function pointer types for general functions
 * and class member functions.
 */
class MemberFunction : public Function {
    static const std::map<std::string, std::string> special_functions_;
 public:
  MemberFunction(CXCursor c, LanguageObject *p)
      : Function(c, p),
        is_const_(clang_CXXMethod_isConst(c)),
        is_static_(clang_CXXMethod_isStatic(c))

        {}

  /** Return spelling of corresponding function pointer type.
     *
     * Prints the function type, which is required to identify the function
     * from potential overloads.
     *
     * @return The spelling of a pointer to the function.
     */
  std::string get_pointer_type_spelling() const {
    std::stringstream ss;
    ss << return_type_.get_qualified_name() << "(";
    // Class qualifier only need for non-static functions.
    if (!is_static_) {
      ss << parent_->get_qualified_name() << "::";
    }
    ss << "*)(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
      ss << arguments_[i]->get_type().get_qualified_name();
      if (i < arguments_.size() - 1) {
        ss << ", ";
      }
    }
    ss << ")";
    if (is_const_) {
      ss << "const ";
    }
    return ss.str();
  }

  /** Clone member function object.
   *
   * Creates a clone of the function object on the heap and updates
   * its parent pointer.
   *
   * @param p The new parent of the clone object.
   * @return Smart pointer to cloned  function object.
   */
  std::shared_ptr<MemberFunction> clone(LanguageObject *p = nullptr) {
    auto f = pxx::cxx::clone<MemberFunction>(*this, p);
    std::transform(f->arguments_.begin(),
                   f->arguments_.end(),
                   f->arguments_.begin(),
                   [&f](auto a) { return a->clone(f.get()); });
    f->return_type_ = *f->return_type_.clone(f.get());
    return f;
  }

  std::string get_python_name() const {
    auto search = special_functions_.find(name_);
    if (search != special_functions_.end()) {
      return search->second;
    }
    return name_;
  }

 private:
  bool is_const_;
  bool is_static_;
};

/** JSON serialization of MemberFunction
 *
 * @param j JSON handle serving as destination.
 * @param lo The language object to serialize.
 */
void to_json(json &j, const MemberFunction &f) {
  to_json(j, static_cast<Function>(f));
  j["name"] = f.get_python_name();
  j["pointer_type"] = f.get_pointer_type_spelling();
}

/** Class constructor.
 *
 * Specialization of MemberFunction class to represent constructors.
 */
class Constructor : public MemberFunction {
 public:
  Constructor(CXCursor c, LanguageObject *p) : MemberFunction(c, p) {
        if (p) {
            auto scope = p->get_scope();
            std::cout << "CONSTRUCTOR " << name_ << ", scope: " << scope->get_name() << std::endl;
            auto replacements = scope->get_type_replacements();
            auto names = std::get<0>(replacements);
            auto values = std::get<1>(replacements);
            for (size_t i = 0; i < names.size(); ++i) {
                std::cout << names[i] << " // " << values[i] << std::endl;
            }
            std::cout << std::endl;
        }

    }
  /** Clone constructor.
    *
    * Creates a clone of the function object on the heap and updates
    * its parent pointer.
    *
    * @param p The new parent of the clone object.
    * @return Smart pointer to cloned  function object.
    */
  std::shared_ptr<Constructor> clone(LanguageObject *p = nullptr) {
    auto f = pxx::cxx::clone<Constructor>(*this, p);
    std::transform(f->arguments_.begin(),
                   f->arguments_.end(),
                   f->arguments_.begin(),
                   [&f](auto a) { return a->clone(f.get()); });
    f->return_type_ = *f->return_type_.clone(f.get());
    return f;
  }
};

std::ostream &operator<<(std::ostream &stream, const Function &f) {
  stream << f.name_ << "(";
  for (size_t i = 0; i < f.arguments_.size(); ++i) {
    auto &p = f.arguments_[i];
    stream << p->get_typename();
    if (i < f.arguments_.size() - 1) {
      stream << ", ";
    }
  }
  stream << ") exported = " << f.get_export_settings().exp << std::endl;
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/** C++ Classes
 *
 * Class to represent C++ classes in the AST. A class holds constructors,
 * member functions, data members, as well as type aliases and type defs.
 */
class Class : public LanguageObject {
 public:
  /** Create class from libclang cursor.
     *
     * @param c libclang cursor of kind CXCursor_ClassDecl representing
     * the class.
     * @param p Pointer to parent object.
     */
  Class(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p),
        scope_(name_, p ? p->get_scope() : nullptr),
        usr_(to_string(clang_getCursorUSR(c))),
        type_(c, p->get_scope()) {
    parent_scope_->add_type_alias(name_, get_qualified_name());
    parent_scope_->register_type(usr_, &type_);
  }

  void set_name(std::string n) {
    std::string old_name = name_;
    LanguageObject::set_name(n);
    scope_.set_name(n);
    parent_->get_scope()->add_type_alias(old_name, get_qualified_name());
  }

  void set_parent(LanguageObject *p) {
    this->LanguageObject::set_parent(p);
    scope_.set_parent(p->get_scope());
  }

  Scope *get_scope() { return &scope_; }

  /** Create clone of class object.
     *
     * Clones the class object and updates the parent pointer.
     * @param parent The new parent of the class object.
     * @return The cloned class object.
     */
  std::shared_ptr<Class> clone(LanguageObject *parent = nullptr) {
    std::shared_ptr<Class> c = pxx::cxx::clone(*this, parent);
    c->parent_->get_scope()->update_type(usr_, &c->type_);
    std::transform(c->constructors_.begin(),
                   c->constructors_.end(),
                   c->constructors_.begin(),
                   [c](auto a) { return a->clone(c.get()); });
    std::transform(c->member_functions_.begin(),
                   c->member_functions_.end(),
                   c->member_functions_.begin(),
                   [c](auto a) { return a->clone(c.get()); });
    std::transform(c->data_members_.begin(),
                   c->data_members_.end(),
                   c->data_members_.begin(),
                   [c](auto a) { return a->clone(c.get()); });
    std::transform(c->type_aliases_.begin(),
                   c->type_aliases_.end(),
                   c->type_aliases_.begin(),
                   [c](auto a) { return a->clone(c.get()); });
    std::transform(c->template_aliases_.begin(),
                   c->template_aliases_.end(),
                   c->template_aliases_.begin(),
                   [c](auto a) { return a->clone(c.get()); });
    return c;
  }

  /** Add constructor to class.
     *
     * Used for parsing the class from the clang AST.
     *
     * @param c The constructor object representing the parsed constructor.
     */
  void add(std::shared_ptr<Constructor> c) { constructors_.push_back(c); }
  /** Add member function to class.
   *
   * Used for parsing the class from the clang AST.
   *
   * @param c The member function object representing the parsed member function.
   */
  void add(std::shared_ptr<MemberFunction> c) {
    member_functions_.push_back(c);
  }
  /** Add member data member to class.
   *
   * Used for parsing the class from the clang AST.
   *
   * @param c The member data member object representing the parsed data member.
   */
  void add(std::shared_ptr<DataMember> m) { data_members_.push_back(m); }

  void add(std::shared_ptr<TypeAlias> t) { type_aliases_.push_back(t); }

  void add(std::shared_ptr<TemplateAlias> t) { template_aliases_.push_back(t); }

  void replace_template_variables(
      const std::vector<std::string> &template_names,
      const std::vector<std::string> &template_arguments) {
    for (auto &c : constructors_) {
      c->replace_template_variables(template_names, template_arguments);
    }
    for (auto &m : member_functions_) {
      m->replace_template_variables(template_names, template_arguments);
    }
    for (auto &d : data_members_) {
      d->replace_template_variables(template_names, template_arguments);
    }
    for (auto &t : type_aliases_) {
      t->replace_template_variables(template_names, template_arguments);
    }
    for (auto &t : template_aliases_) {
      t->replace_template_variables(template_names, template_arguments);
    }
  }

  /** Get exported class methods.
   * @return Vector containing the class methods exported by this class.
   */
  std::vector<std::shared_ptr<MemberFunction>> get_exported_methods() const {
    std::vector<std::shared_ptr<MemberFunction>> exports;
    std::copy_if(member_functions_.begin(),
                 member_functions_.end(),
                 std::back_inserter(exports),
                 [](std::shared_ptr<Function> f) {
                   return f->get_export_settings().exp;
                 });
    return exports;
  }

  /** Get exported constructors
   * @return Vector containing the constructors exported by this class.
   */
  std::vector<std::shared_ptr<Constructor>> get_exported_constructors() const {
    std::vector<std::shared_ptr<Constructor>> exports;
    std::copy_if(constructors_.begin(),
                 constructors_.end(),
                 std::back_inserter(exports),
                 [](std::shared_ptr<Function> f) {
                   return f->get_export_settings().exp;
                 });
    return exports;
  }

  /** Get exported variables
   * @return Vector containing the data members exported by this class.
   */
  std::vector<std::shared_ptr<DataMember>> get_exported_data_members() const {
    std::vector<std::shared_ptr<DataMember>> exports;
    std::copy_if(data_members_.begin(),
                 data_members_.end(),
                 std::back_inserter(exports),
                 [](std::shared_ptr<DataMember> v) {
                   return v->get_export_settings().exp;
                 });
    return exports;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Class &cl);
  friend void to_json(json &j, const Class &c);

 private:
  Scope scope_;
  std::string usr_;
  Type type_;
  std::vector<std::shared_ptr<Constructor>> constructors_;
  std::vector<std::shared_ptr<MemberFunction>> member_functions_;
  std::vector<std::shared_ptr<DataMember>> data_members_;
  std::vector<std::shared_ptr<TypeAlias>> type_aliases_;
  std::vector<std::shared_ptr<TemplateAlias>> template_aliases_;
};

using ClassTemplate = Template<Class>;
using ClassTemplateSpecialization = TemplateSpecialization<Class>;

void to_json(json &j, const Class &c) {
  to_json(j, static_cast<LanguageObject>(c));
  j["constructors"] = c.get_exported_constructors();
  j["methods"] = c.get_exported_methods();
  j["data_members"] = c.get_exported_data_members();
}

std::ostream &operator<<(std::ostream &stream, const Class &cl) {
  stream << "Class: " << cl.name_ << std::endl;
  stream << "Constructors:" << std::endl;
  for (auto &&f : cl.constructors_) {
    stream << "\t" << *f;
  }
  stream << "Methods:" << std::endl;
  for (auto &&f : cl.member_functions_) {
    stream << "\t" << *f;
  }
  stream << "Data members:" << std::endl;
  for (auto &&v : cl.data_members_) {
    stream << "\t" << *v;
  }

  stream << "\t"
         << "Exported: " << cl.settings_.exp << std::endl;
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////////////

class Namespace : public LanguageObject {
 public:
  Namespace(CXCursor cursor, LanguageObject *parent)
      : LanguageObject(cursor, parent),
        scope_(name_, parent ? parent->get_scope() : nullptr) {
    if (parent) {
        std::cout << "Adding namespace: " << get_qualified_name() << " / " << parent->get_name() << std::endl;
      parent->get_scope()->add_type_alias(name_, get_qualified_name());
    }
  }
  void set_parent(LanguageObject *p) {
    this->LanguageObject::set_parent(p);
    scope_.set_parent(p->get_scope());
  }

  Scope *get_scope() { return &scope_; }

  /// Add class to namespace.
  void add(std::shared_ptr<Class> cl) { classes_.push_back(cl); }
  /// Add class template to namespace.
  void add(std::shared_ptr<ClassTemplate> cl) {
    class_templates_.push_back(cl);
  }
  /// Add function to namespace.
  void add(std::shared_ptr<Function> function) {
    functions_.push_back(function);
  }
  /// Add function template to namespace.
  void add(std::shared_ptr<FunctionTemplate> function) {
    function_templates_.push_back(function);
  }
  /// Add nested namespace.
  void add(std::shared_ptr<Namespace> ns) {
    std::string name = ns->get_name();
    auto pred = [&name](std::shared_ptr<Namespace> ns_ptr) {
      return ns_ptr->get_name() == name;
    };
    auto found = std::find_if(namespaces_.begin(), namespaces_.end(), pred);
    if (found != namespaces_.end()) {
      (*found)->join(*ns);
    } else {
      namespaces_.push_back(ns);
    }
  }
  /// Add enum to namespace.
  void add(std::shared_ptr<Enum> e) { enums_.push_back(e); }

  std::vector<std::shared_ptr<Namespace>> get_namespaces() const {
    return namespaces_;
  }

  std::vector<std::shared_ptr<Class>> get_classes() const { return classes_; }

  std::vector<std::shared_ptr<ClassTemplate>> get_class_templates() const {
    return class_templates_;
  }

  std::vector<std::shared_ptr<Function>> get_functions() const {
    return functions_;
  }

  std::vector<std::shared_ptr<FunctionTemplate>> get_function_templates()
      const {
    return function_templates_;
  }

  std::vector<std::shared_ptr<Enum>> get_enums() const {
      return enums_;
  }

  void join(Namespace &other) {
    scope_.join(other.scope_);

    auto other_classes = other.get_classes();
    detail::set_parents(other_classes, this);
    classes_.insert(classes_.end(), other_classes.begin(), other_classes.end());

    auto other_class_templates = other.get_class_templates();
    detail::set_parents(other_class_templates, this);
    class_templates_.insert(class_templates_.end(),
                            other_class_templates.begin(),
                            other_class_templates.end());
    auto other_functions = other.get_functions();
    detail::set_parents(other_functions, this);
    functions_.insert(
        functions_.end(), other_functions.begin(), other_functions.end());
    auto other_function_templates = other.get_function_templates();
    detail::set_parents(other_function_templates, this);
    function_templates_.insert(function_templates_.end(),
                               other_function_templates.begin(),
                               other_function_templates.end());
    auto other_namespaces = other.get_namespaces();
    detail::set_parents(other_namespaces, this);
    namespaces_.insert(
        namespaces_.end(), other_namespaces.begin(), other_namespaces.end());

    auto other_enums = other.get_enums();
    detail::set_parents(other_enums, this);
    enums_.insert(
        enums_.end(), other_enums.begin(), other_enums.end());
  }

  std::vector<std::shared_ptr<Function>> get_exported_functions() const {
    std::vector<std::shared_ptr<Function>> exports;
    std::copy_if(functions_.begin(),
                 functions_.end(),
                 std::back_inserter(exports),
                 [](std::shared_ptr<Function> f) {
                   return f->get_export_settings().exp;
                 });
    for (auto &&n : namespaces_) {
      auto nested_functions = n->get_exported_functions();
      exports.insert(
          exports.end(), nested_functions.begin(), nested_functions.end());
    }

    return exports;
  }

  std::vector<std::shared_ptr<Class>> get_exported_classes() const {
    std::vector<std::shared_ptr<Class>> exports;
    std::copy_if(
        classes_.begin(),
        classes_.end(),
        std::back_inserter(exports),
        [](std::shared_ptr<Class> c) { return c->get_export_settings().exp; });
    for (auto &&n : namespaces_) {
      auto nested_classes = n->get_exported_classes();
      exports.insert(
          exports.end(), nested_classes.begin(), nested_classes.end());
    }
    return exports;
  }

  std::vector<std::shared_ptr<Function>> get_function_template_instances()
      const {
    std::vector<std::shared_ptr<Function>> exports;
    for (auto &&ft : function_templates_) {
      auto instances = ft->get_instances();
      exports.insert(exports.end(), instances.begin(), instances.end());
    }
    for (auto &&n : namespaces_) {
      auto nested_instances = n->get_function_template_instances();
      exports.insert(
          exports.end(), nested_instances.begin(), nested_instances.end());
    }
    return exports;
  }

  std::vector<std::shared_ptr<Class>> get_class_template_instances() const {
    std::vector<std::shared_ptr<Class>> exports{};
    for (auto &&ft : class_templates_) {
      auto instances = ft->get_instances();
      exports.insert(exports.end(), instances.begin(), instances.end());
    }
    for (auto &&n : namespaces_) {
      auto nested_instances = n->get_class_template_instances();
      exports.insert(
          exports.end(), nested_instances.begin(), nested_instances.end());
    }
    return exports;
  }

  std::vector<std::shared_ptr<Enum>> get_exported_enums() const {
  std::vector<std::shared_ptr<Enum>> exports{};
    std::copy_if(
        enums_.begin(),
        enums_.end(),
        std::back_inserter(exports),
        [](std::shared_ptr<Enum> e) { return e->get_export_settings().exp; });
    for (auto &&n : namespaces_) {
      auto nested_enums = n->get_exported_enums();
      exports.insert(exports.end(), nested_enums.begin(), nested_enums.end());
    }
    return exports;
  }

  bool uses_std() const {
    auto pred = [](std::shared_ptr<Namespace> ns_ptr) {
      return ns_ptr->get_name() == "std";
    };
    return (std::find_if(namespaces_.begin(), namespaces_.end(), pred) !=
            namespaces_.end());
  }

  bool uses_eigen() const {
    auto pred = [](std::shared_ptr<Namespace> ns_ptr) {
      return ns_ptr->get_name() == "Eigen";
    };
    return (std::find_if(namespaces_.begin(), namespaces_.end(), pred) !=
            namespaces_.end());
  }

  friend std::ostream &operator<<(std::ostream &stream, const Namespace &t);

 protected:
  Scope scope_;
  std::vector<std::shared_ptr<Class>> classes_;
  std::vector<std::shared_ptr<ClassTemplate>> class_templates_;
  std::vector<std::shared_ptr<Function>> functions_;
  std::vector<std::shared_ptr<FunctionTemplate>> function_templates_;
  std::vector<std::shared_ptr<Namespace>> namespaces_;
  std::vector<std::shared_ptr<Enum>> enums_;
};

std::ostream &operator<<(std::ostream &stream, const Namespace &ns) {
  stream << "::: C++ namespace : "
         << " " << ns.name_ << " :::" << std::endl;
  stream << "Defined functions:" << std::endl;
  for (auto &&f : ns.get_exported_functions()) {
    std::cout << std::endl;
    std::cout << *f;
  }
  stream << "Defined classes:" << std::endl;
  for (auto &&cl : ns.get_exported_classes()) {
    std::cout << std::endl;
    std::cout << *cl;
  }
  stream << "Defined namespaces:" << std::endl;
  for (auto &&n : ns.get_namespaces()) {
    std::cout << std::endl;
    std::cout << *n;
  }
  return stream;
}

void to_json(json &j, const Namespace &t) {
  j["classes"] = t.get_exported_classes();
  j["functions"] = t.get_exported_functions();
  j["function_template_instances"] = t.get_function_template_instances();
  j["class_template_instances"] = t.get_class_template_instances();
  j["enums"] = t.get_exported_enums();
}
////////////////////////////////////////////////////////////////////////////////
// Parsing functions.
////////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T>
struct Parser;
}

/** Parse generic language object.
 *
 * Creates a new language object and parses its children.
 *
 * @tparam T The language object class to parse.
 * @param c The clang AST pointer to the object that should be parsed.
 * @param ps Shared pointer to the parent scope.
 * @return Shared pointer to the newly created language object.
 */
template <typename T>
static std::shared_ptr<T> parse(CXCursor c, LanguageObject *ps) {
  auto lo = std::make_shared<T>(c, ps);
  clang_visitChildren(c, detail::Parser<T>::parse_impl, lo.get());
  return lo;
}

/** Parse top-level language object.
 *
 * Creates a top-level language object and parses its children.
 *
 * @tparam T The language object class to parse.
 * @param c The clang AST pointer to the object that should be parsed.
 * @param ps Shared pointer to the parent scope.
 * @return Shared pointer to the newly created language object.
 */
template <typename T>
static std::shared_ptr<T> parse(CXCursor c, ExportSettings s) {
  auto lo = std::make_shared<T>(c, nullptr);
  lo->set_export_settings(s);
  clang_visitChildren(c, detail::Parser<T>::parse_impl, lo.get());
  return lo;
}

namespace detail {
template <typename T>
struct Parser<Template<T>> {
  /// parser specialization for templates.
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor /*parent*/,
                                       CXClientData client_data) {
    Template<T> *t = reinterpret_cast<Template<T> *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
      case CXCursor_TemplateTypeParameter: {
        t->add(std::make_shared<LanguageObject>(c, t->get_base()));
      } break;
      case CXCursor_NonTypeTemplateParameter: {
        t->add(parse<LanguageObject>(c, t->get_base()));
      } break;
      default:
        break;
    }
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<LanguageObject> {
  static CXChildVisitResult parse_impl(CXCursor /*c*/,
                                       CXCursor /*p*/,
                                       CXClientData /*client_data*/) {
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<Namespace> {
  /// parser specialization for namespaces.
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor /*parent*/,
                                       CXClientData client_data) {
    Namespace *ns = reinterpret_cast<Namespace *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
      case CXCursor_ClassDecl: {
        ns->add(parse<Class>(c, ns));
      } break;
      case CXCursor_FunctionDecl: {
        ns->add(parse<Function>(c, ns));
      } break;
      case CXCursor_FunctionTemplate: {
        auto f = parse<Function>(c, ns);
        auto t = std::make_shared<FunctionTemplate>(f);
        clang_visitChildren(c, Parser<FunctionTemplate>::parse_impl, t.get());
        ns->add(t);
      } break;
      case CXCursor_ClassTemplatePartialSpecialization: {
        auto f = parse<Class>(c, ns);
        auto t = std::make_shared<ClassTemplateSpecialization>(c, f);
        clang_visitChildren(c, Parser<ClassTemplate>::parse_impl, t.get());
        ns->add(t);
      } break;
      case CXCursor_ClassTemplate: {
        if (clang_isCursorDefinition(c)) {
          auto f = parse<Class>(c, ns);
          auto t = std::make_shared<ClassTemplate>(f);
          clang_visitChildren(c, Parser<ClassTemplate>::parse_impl, t.get());
          ns->add(t);
        }
      } break;
      case CXCursor_Namespace: {
        ns->add(parse<Namespace>(c, ns));
      } break;
      case CXCursor_EnumDecl: {
        ns->add(parse<Enum>(c, ns));
      } break;
      default:
        break;
    }
    return CXChildVisit_Continue;
  }
};  // namespace detail

/// parser specialization for classes.
template <>
struct Parser<Class> {
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor /*parent*/,
                                       CXClientData client_data) {
    Class *cl = reinterpret_cast<Class *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
      case CXCursor_Constructor: {
        cl->add(parse<Constructor>(c, cl));
      } break;
    case CXCursor_FunctionTemplate:
      case CXCursor_CXXMethod: {
        cl->add(parse<MemberFunction>(c, cl));
      } break;
      case CXCursor_FieldDecl: {
        cl->add(parse<DataMember>(c, cl));
      } break;
      case CXCursor_TypeAliasDecl: {
        cl->add(parse<TypeAlias>(c, cl));
      } break;
      case CXCursor_TypeAliasTemplateDecl: {
        cl->add(std::make_shared<TemplateAlias>(c, cl));
      } break;
      default:
        break;
    }
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<Variable> {
  static CXChildVisitResult parse_impl(CXCursor /*c*/,
                                       CXCursor /*p*/,
                                       CXClientData /*client_data*/) {
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<TypeAlias> {
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor /*p*/,
                                       CXClientData client_data) {
    TypeAlias *ta = reinterpret_cast<TypeAlias *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
      case CXCursor_TypeRef: {
        CXCursor cr = clang_getCursorReferenced(c);
        std::string usr = to_string(clang_getCursorUSR(cr));
        ta->update_reference(ta->get_scope()->lookup_type(usr));
      } break;
      default:
        break;
    }
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<DataMember> {
  static CXChildVisitResult parse_impl(CXCursor /*c*/,
                                       CXCursor /*p*/,
                                       CXClientData /*client_data*/) {
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<Function> {
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor /*parent*/,
                                       CXClientData client_data) {
    Function *f = reinterpret_cast<Function *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
      case CXCursor_ParmDecl: {
        f->add(parse<Variable>(c, f));
      } break;
      default:
        break;
    }
    return CXChildVisit_Continue;
  }
};

template <>
struct Parser<MemberFunction> {
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor p,
                                       CXClientData client_data) {
    return Parser<Function>::parse_impl(c, p, client_data);
  }
};

template <>
struct Parser<Constructor> {
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor p,
                                       CXClientData client_data) {
    return Parser<Function>::parse_impl(c, p, client_data);
  }
};

template <>
struct Parser<Enum> {
  /// parser specialization for enums.
  static CXChildVisitResult parse_impl(CXCursor c,
                                       CXCursor /*parent*/,
                                       CXClientData client_data) {
    Enum *e = reinterpret_cast<Enum *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
      case CXCursor_EnumConstantDecl: {
        e->add(parse<LanguageObject>(c, e));
      } break;
    }
    return CXChildVisit_Continue;
  }
};

}  // namespace detail

const std::map<std::string, std::string> MemberFunction::special_functions_ =
    {std::make_pair("operator+", "__add__"),
     std::make_pair("operator+=", "__iadd__"),
     std::make_pair("operator*", "__mul__"),
     std::make_pair("operator*=", "__imul__"),
     std::make_pair("operator/", "__div__"),
     std::make_pair("operator/=", "__idiv__")};

}  // namespace cxx
}  // namespace pxx

#endif
