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

#include <pxx/comment_parser.h>
#include <pxx/utils.h>

#include <inja/inja.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace pxx {
namespace cxx {

using pxx::comment_parser::ExportSettings;
using pxx::comment_parser::CommentParser;
using json = nlohmann::json;

namespace detail {

using pxx::to_string;

/** Replace names.
 *
 * Textual replacement of names.
 *
 * @param tname The type name containing template variables.
 * @param template_names The names of the template variables.
 * @param template_arguments The replacements for each variable.
 * @return The given typename with all template variables replaced
 *   with the corresponding types or variables.
 */
std::string replace_names(std::string name,
                          const std::vector<std::string> &names,
                          const std::vector<std::string> &values) {
  std::regex re("[a-zA-Z_][a-zA-Z0-9_]*");

  // Search for valid C++ identifiers in name and replace.
  std::string input = name;
  std::smatch match;
  std::regex_search(name, match, re);
  std::stringstream output{};

  while (!match.empty()) {
    std::string ms = match.str();

    // Copy what didn't match.
    output << match.prefix();

    bool matched = false;
    for (int i = 0; i < static_cast<int>(names.size()); ++i) {
      const std::string &n = names[i];
      if (n == ms) {
        // Replace match.
        output << values[i];
        matched = true;
        break;
      }
    }
    // If didn't match, keep string.
    if (!matched) {
      output << ms;
    }
    input = match.suffix();
    std::regex_search(input, match, re);
  }
  output << input;
  return output.str();
}

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


}  // namespace detail

template <typename T>
void to_json(json &j, std::shared_ptr<T> pt) {
  if (pt) {
    to_json(j, *pt);
  }
}

/**
 * A scope holds names that need to be resolved in order to identify
 * an object in another scope.
 */
class Scope {
 public:
  Scope(std::string name, Scope *parent_scope)
      : name_(name), parent_scope_(parent_scope) {}

  std::string get_name() {return name_;}
  void set_name(std::string s) {name_ = s;}
  void add_name(std::string name) { names_.push_back(name); }

  size_t lookup_name(std::string name) {
    size_t count = 0;
    auto end = names_.end();
    auto it = std::find(names_.begin(), end, name);
    while (it != end) {
      ++count;
      ++it;
      it = std::find(it, end, name);
    }
    return count;
  }

  void add_type_name(std::string name, std::string value) {
    type_names_.push_back(name);
    type_replacements_.push_back(value);
  }

  std::pair<std::vector<std::string>, std::vector<std::string>>
  get_type_replacements() {
    return std::make_pair(type_names_, type_replacements_);
  }

  std::string get_prefix() {
      if (name_ != "") {
          return name_ + "::";
      }
      return "";
  }

 protected:
  std::string name_;
  Scope *parent_scope_;
  std::vector<std::string> names_ = {};
  std::vector<std::string> type_names_ = {};
  std::vector<std::string> type_replacements_ = {};
};

/** Any C++ language object.
 *
 * Each language object has two types of name. Its normal name,
 * or spelling, which is the objects name as it appears in the code,
 * and the qualified name, which is the name that identifies the the
 * object at root scope.
 *
 * Each language object also has a parent scope, which it needs to know
 * in order to determine its qualified name.
 *
 * Additionally, each language object has export settings that define
 * their behavior during interface creation.
 *
 */
class LanguageObject {
 public:
  /** Create new language object.
     *
     * @param cursor The Clang AST cursor representing the object.
     * @param parent_scope The parent scope of the language object.
     */
  LanguageObject(CXCursor cursor, LanguageObject *parent = nullptr)
      : name_(to_string(clang_getCursorSpelling(cursor))),
        parent_(parent),
        scope_(parent ? name_ : "", parent ? parent->get_scope() : nullptr) {

        if (parent) {
          settings_ = parent->get_export_settings();
        }

        export_name_ = name_;

        if (clang_isDeclaration(clang_getCursorKind(cursor))) {
          auto s = to_string(clang_Cursor_getRawCommentText(cursor));
          CommentParser cp(s, settings_);
          settings_ = cp.settings;
    }

  }

  /** Spelling of language object.
     * @return The name of the object as it appears in the code.
     */
  std::string get_name() const { return name_; }

  void set_name(std::string name) {
      scope_.set_name(name);
      name_ = name;
  }

  void set_parent(LanguageObject *lo) {
      parent_ = lo;
  }

  /** Qualified name of language object.
     *
     * The qualified name is the name of the object that identifies it at root scope.
     *
     * @return The qualified name with all dependent parts are resolved.
     */
  std::string get_qualified_name() const {
      std::string prefix = "";
      if (parent_) {
          Scope *ps = parent_->get_scope();
          if (ps) {
              prefix = ps->get_prefix();
          }
      }
      return prefix + name_;
  }

  std::string get_export_name() const {return export_name_;}
  void set_export_name(std::string name) {export_name_ = name;}

  /// Get the object's parent scope.
  Scope *get_scope() { return &scope_; }
  const Scope *get_scope() const { return &scope_; }

  /// Get the object's export settings.
  ExportSettings get_export_settings() const { return settings_; }
  void set_export_settings(ExportSettings settings) {settings_ = settings;}

 protected:
  std::string name_;
  std::string export_name_;
  LanguageObject *parent_;
  Scope scope_;
  ExportSettings settings_;
};

template <typename T>
std::shared_ptr<T> clone(const T &t, LanguageObject *p) {
    std::shared_ptr<T> a = std::make_shared<T>(t);
    if (!p) {
        a->set_parent(p);
    }
    return a;
}

void to_json(json &j, LanguageObject lo) {
  j["name"] = lo.get_name();
  j["qualified_name"] = lo.get_qualified_name();
  j["export_name"] = lo.get_export_name();
}

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

/** A C++ type.
 * Represents types associated with C++ language objects.
 */
class Type {
 public:
  /** Create Type.
     * @param t CXType to represent.
     */
    Type(CXType t, Scope *p) : type_(t), scope_(p) {
    name_ = to_string(clang_getTypeSpelling(type_));

    std::vector<std::string> names, values;
    std::tie(names, values) = p->get_type_replacements();
    qualified_name_ = detail::replace_names(name_, names, values);

    is_pointer_ = (type_.kind == CXType_Pointer);
    is_lvalue_reference_ = (type_.kind == CXType_LValueReference);
    is_rvalue_reference_ = (type_.kind == CXType_RValueReference);
    is_const_ = clang_isConstQualifiedType(type_);
  }

  Type(CXCursor c, Scope *p) : Type(clang_getCursorType(c), p) {}

  /// The type's name.
  std::string get_name() const { return name_; }

  /// The type's name that identifies it at root scope.
  std::string get_qualified_name() const { return qualified_name_; }

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
    qualified_name_ = detail::replace_names(qualified_name_, names, values);
  }

  /// Is the type const qualified?
  bool is_const() const { return is_const_; }
  /// Is the type a pointer?
  bool is_pointer() const { return is_pointer_; }
  /// Is the type a l-value reference?
  bool is_lvalue_reference() const { return is_lvalue_reference_; }
  /// Is the type a r-value reference?
  bool is_rvalue_reference() const { return is_rvalue_reference_; }
  /// Is the type an Eigen type?

  void set_scope(Scope *s) {scope_ = s;}

  friend std::ostream &operator<<(std::ostream &stream, const Type &t);

 protected:
  std::string name_;
  std::string qualified_name_;
  CXType type_;
  bool is_const_;
  bool is_pointer_;
  bool is_lvalue_reference_;
  bool is_rvalue_reference_;
  Scope *scope_;
};

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
  Variable(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p), type_(c, p->get_scope()) {}

  std::string get_typename() const { return type_.get_name(); }

  bool is_const() const { return type_.is_const(); }
  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const Variable &v);

  std::shared_ptr<Variable> clone(LanguageObject *p) {
      std::shared_ptr<Variable> v = std::make_shared<Variable>(*this);
      set_parent(p);
      type_.set_scope(p->get_scope());
  }

  void replace_template_variables(
      const std::vector<std::string> &template_names,
      const std::vector<std::string> &template_values) {
    type_.replace_template_variables(template_names, template_values);
  }

 protected:
  std::string typename_;
  Type type_;
};

void to_json(json &j, const Variable &v) {
  to_json(j, static_cast<LanguageObject>(v));
  j["type"] = v.type_;
  j["const"] = v.type_.is_const();
}

std::ostream &operator<<(std::ostream &stream, const Variable &v) {
  stream << v.typename_ << " " << v.name_ << std::endl;
  return stream;
}

class DataMember : public Variable {
 public:
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

  std::string get_type_spelling() const { return typename_; }

  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const DataMember &v);

 private:
  detail::access_specifier_t access_specifier_;
  detail::storage_class_t storage_class_;
  bool valid_ = true;
};

void to_json(json &j, const DataMember &v) {
  to_json(j, static_cast<Variable>(v));
  j["access"] = to_string(v.access_specifier_);
  j["storage"] = to_string(v.storage_class_);
}

////////////////////////////////////////////////////////////////////////////////
// Cxx Templates
////////////////////////////////////////////////////////////////////////////////

template <typename Base>
class Template {
 public:
  Template(std::shared_ptr<Base> t) : template_(t) {}

  Base *get_base() { return template_.get(); }

  void add(std::shared_ptr<LanguageObject> lo) { arguments_.push_back(lo); }

  std::string get_template_name(std::vector<std::string> template_replacements) const {
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

  std::vector<std::shared_ptr<Base>> get_instances() const {
    std::vector<std::string> argument_names;
    for (auto &ta : arguments_) {
      argument_names.push_back(ta->get_name());
    }
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
      template_instance->replace_template_variables(argument_names, std::get<1>(is));
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

 protected:
  std::shared_ptr<Base> template_ = nullptr;
  std::vector<std::shared_ptr<LanguageObject>> arguments_ = {};
};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

/** A C++ Function.
 *
 * Represents independent functions defined at namespace scope.
 */
class Function : public LanguageObject {
 public:
  Function(CXCursor c, LanguageObject *p)
      : LanguageObject(c, p),
        return_type_(clang_getResultType(clang_getCursorType(c)), p->get_scope()) {
        p->get_scope()->add_name(name_);
    }

  void add(std::shared_ptr<Variable> a) { arguments_.push_back(a); }

  void replace_template_variables(
      const std::vector<std::string> &template_names,
      const std::vector<std::string> &template_values) {
    for (auto &p : arguments_) {
      p->replace_template_variables(template_names, template_values);
    }
    return_type_.replace_template_variables(template_names, template_values);
  }

  std::shared_ptr<Function> clone(LanguageObject *p = nullptr) {
    auto f = pxx::cxx::clone<Function>(*this, p);
    std::transform(f->arguments_.begin(),
                   f->arguments_.end(),
                   f->arguments_.begin(),
                   [&f](auto a) {return a->clone(f);});
    f->return_type_->set_scope(f->get_scope());
    return f;
  }

  std::string get_pointer_type_spelling() const {
    std::stringstream ss;
    ss << "(" << return_type_.get_qualified_name() << ")(*)(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
      ss << arguments_[i]->get_qualified_name();
      if (i < arguments_.size() - 1) {
        ss << ", ";
      }
    }
    ss << ")";
    return ss.str();
  }

  bool has_overload() const {
      return (parent_->get_scope()->lookup_name(name_) > 1);
  }

  friend std::ostream &operator<<(std::ostream &stream, const Function &cl);
  friend void to_json(json &j, const Function &f);

 protected:
  std::vector<std::shared_ptr<Variable>> arguments_;
  Type return_type_;
};

using FunctionTemplate = Template<Function>;

void to_json(json &j, const Function &f) {
    to_json(j, static_cast<LanguageObject>(f));
    j["arguments"] = f.arguments_;
    j["return_type"] = f.return_type_;
    j["has_overload"] = f.has_overload();
    j["pointer_type"] = f.get_pointer_type_spelling();
}

class MemberFunction : public Function {
 public:
  MemberFunction(CXCursor c, LanguageObject *p) : Function(c, p) {}

  std::string get_pointer_type_spelling() const {
    std::stringstream ss;
    ss << return_type_.get_qualified_name()
       << "(" << parent_->get_qualified_name() << "::*)(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
      ss << arguments_[i]->get_qualified_name();
      if (i < arguments_.size() - 1) {
        ss << ", ";
      }
    }
    ss << ")";
    return ss.str();
  }
};

void to_json(json &j, const MemberFunction &f) {
    to_json(j, static_cast<Function>(f));
    j["pointer_type"] = f.get_pointer_type_spelling();
}

class Constructor : public MemberFunction {
 public:
  Constructor(CXCursor c, LanguageObject *p) : MemberFunction(c, p) {}
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
  Class(CXCursor c, LanguageObject *p) : LanguageObject(c, p) {}

std::shared_ptr<Class> clone(LanguageObject *parent=nullptr) {
    std::shared_ptr<Class> c = pxx::cxx::clone(*this, parent);
    std::transform(c->constructors_.begin(),
                   c->constructors_.end(),
                   c->constructors_.begin(),
                   [c](auto a) {return pxx::cxx::clone(*a, c.get());});
    std::transform(c->member_functions_.begin(),
                   c->member_functions_.end(),
                   c->member_functions_.begin(),
                   [c](auto a) {return pxx::cxx::clone(*a, c.get());});
    std::transform(c->data_members_.begin(),
                   c->data_members_.end(),
                   c->data_members_.begin(),
                   [c](auto a) {return pxx::cxx::clone(*a, c.get());});
    return c;
  }

  void add(std::shared_ptr<Constructor> c) { constructors_.push_back(c); }
  void add(std::shared_ptr<MemberFunction> c) {
    member_functions_.push_back(c);
  }
  void add(std::shared_ptr<DataMember> m) { data_members_.push_back(m); }

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
  }

  std::vector<std::shared_ptr<Function>> get_exported_methods() const {
    std::vector<std::shared_ptr<Function>> exports;
    std::copy_if(member_functions_.begin(),
                 member_functions_.end(),
                 std::back_inserter(exports),
                 [](std::shared_ptr<Function> f) {
                   return f->get_export_settings().exp;
                 });
    return exports;
  }

  std::vector<std::shared_ptr<Function>> get_exported_constructors() const {
    std::vector<std::shared_ptr<Function>> exports;
    std::copy_if(constructors_.begin(),
                 constructors_.end(),
                 std::back_inserter(exports),
                 [](std::shared_ptr<Function> f) {
                   return f->get_export_settings().exp;
                 });
    return exports;
  }

  std::vector<std::shared_ptr<DataMember>> get_exported_variables() const {
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
  std::vector<std::shared_ptr<Constructor>> constructors_;
  std::vector<std::shared_ptr<MemberFunction>> member_functions_;
  std::vector<std::shared_ptr<DataMember>> data_members_;
};

using ClassTemplate = Template<Class>;

void to_json(json &j, const Class &c) {
  to_json(j, static_cast<LanguageObject>(c));
  j["constructors"] = c.constructors_;
  j["methods"] = c.member_functions_;
  j["data_members"] = c.data_members_;
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
      : LanguageObject(cursor, parent) {}

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

  void join(const Namespace &other) {
    auto other_classes = other.get_classes();
    classes_.insert(classes_.end(), other_classes.begin(), other_classes.end());
    auto other_class_templates = other.get_class_templates();
    class_templates_.insert(class_templates_.end(),
                            other_class_templates.begin(),
                            other_class_templates.end());
    auto other_functions = other.get_functions();
    functions_.insert(
        functions_.end(), other_functions.begin(), other_functions.end());
    auto other_function_templates = other.get_function_templates();
    function_templates_.insert(function_templates_.end(),
                               other_function_templates.begin(),
                               other_function_templates.end());
    auto other_namespaces = other.get_namespaces();
    namespaces_.insert(
        namespaces_.end(), other_namespaces.begin(), other_namespaces.end());
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
    std::vector<std::shared_ptr<Class>> exports;
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
  std::vector<std::shared_ptr<Class>> classes_;
  std::vector<std::shared_ptr<ClassTemplate>> class_templates_;
  std::vector<std::shared_ptr<Function>> functions_;
  std::vector<std::shared_ptr<FunctionTemplate>> function_templates_;
  std::vector<std::shared_ptr<Namespace>> namespaces_;
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
      case CXCursor_ClassTemplate: {
        auto f = parse<Class>(c, ns);
        auto t = std::make_shared<ClassTemplate>(f);
        clang_visitChildren(c, Parser<ClassTemplate>::parse_impl, t.get());
        ns->add(t);
      } break;
      case CXCursor_Namespace: {
        ns->add(parse<Namespace>(c, ns));
      } break;
      default:
        break;
    }
    return CXChildVisit_Continue;
  }
};

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
      case CXCursor_CXXMethod: {
        cl->add(parse<MemberFunction>(c, cl));
      } break;
      case CXCursor_FieldDecl: {
        cl->add(parse<DataMember>(c, cl));
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

}  // namespace detail
}  // namespace cxx
}  // namespace pxx

#endif
