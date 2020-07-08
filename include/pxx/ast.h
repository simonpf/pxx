#ifndef __PXX_AST_H__
#define __PXX_AST_H__

#include <clang-c/Index.h>
#include <inja/inja.hpp>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <pxx/utils.h>
#include <pxx/comment_parser.h>

using json = nlohmann::json;

namespace pxx::ast {

using pxx::operator <<;

namespace detail {
    struct AstFormatter {
    AstFormatter(size_t level = 0) : level_(level) {}

        void print(CXCursor c) {
            std::cout << std::setw(level_ * 4) << " " << " + ";
            std::cout << clang_getCursorKindSpelling(clang_getCursorKind(c));
            std::cout << " : " << clang_getCursorSpelling(c);

            CXCursorKind k = clang_getCursorKind(c);
            if (k == CXCursor_ClassDecl) {
                std::cout << "(Ref. template " << clang_getSpecializedCursorTemplate(c) << ")" << std::endl;;

            }
            std::cout << std::endl;
        }

        static CXChildVisitResult traverse(CXCursor c, CXCursor /*p*/, CXClientData d) {
            AstFormatter *formatter = reinterpret_cast<AstFormatter *>(d); formatter->print(c);
            formatter->level_++;
            clang_visitChildren(c, traverse, d);
            formatter->level_--;
            return CXChildVisit_Continue;
        }

        size_t level_ = 0;
    };

}  // namespace detail

using pxx::to_string;

CXChildVisitResult parse_function(CXCursor c, CXCursor parent,
                                  CXClientData client_data);
template <typename Base>
CXChildVisitResult parse_template(CXCursor c, CXCursor parent, CXClientData client_data);
CXChildVisitResult parse_class(CXCursor c, CXCursor parent,
                               CXClientData client_data);
CXChildVisitResult parse_namespace(CXCursor c, CXCursor parent,
                                   CXClientData client_data);

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
    return to_string(s);
}

/** Replace template names.
 *
 * Replaces template variable in a type name.
 *
 * @param tname The type name containing template variables.
 * @param template_names The names of the template variables.
 * @param template_arguments The replacements for each variable.
 * @return The given typename with all template variables replaced
 *   with the corresponding types or variables.
 */
std::string replace_template_variables(
    std::string tname,
    const std::vector<std::string> &template_names,
    const std::vector<std::string> &template_arguments) {
  std::regex re("[a-zA-Z_][a-zA-Z0-9_]*");

  // Search for valid C++ identifiers in name and replace.
  std::string input = tname;
  std::smatch match;
  std::regex_search(tname, match, re);
  std::stringstream output{};

  while (!match.empty()) {
    std::string ms = match.str();

    // Copy what didn't match.
    output << match.prefix();

    bool matched = false;
    for (int i = 0; i < static_cast<int>(template_names.size()); ++i) {
      const std::string &n = template_names[i];
      if (n == ms) {
        // Replace match.
        output << template_arguments[i];
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

////////////////////////////////////////////////////////////////////////////////
// CxxConstruct
////////////////////////////////////////////////////////////////////////////////

class CxxConstruct {
public:
  CxxConstruct(CXCursor c, ExportSettings s = ExportSettings())
      : cursor_(c), settings_(s), name_(pxx::ast::get_name(c)) {}

  CxxConstruct(CXCursor c, const CxxConstruct &p) : cursor_(c) {
    CXString s = clang_getCursorSpelling(c);
    name_ = to_string(s);
    std::string parent_name = p.get_qualified_name();
    if (parent_name != "") {
      qualified_name_ = p.get_qualified_name() + "::" + name_;
    } else {
      qualified_name_ = name_;
    }

    settings_ = p.get_export_settings();
    if (clang_isDeclaration(clang_getCursorKind(c))) {
      auto s = to_string(clang_Cursor_getRawCommentText(c));
      CommentParser cp(s, settings_);
      settings_ = cp.settings;
    }
  }

  std::string get_name() const { return name_; }
  std::string get_qualified_name() const { return qualified_name_; }

  ExportSettings get_export_settings() const { return settings_; }

  friend std::ostream &operator<<(std::ostream &stream, const CxxConstruct &cl);

protected:
  CXCursor cursor_;
  ExportSettings settings_;
  std::string name_;
  std::string qualified_name_;
  bool valid_ = true;
};

////////////////////////////////////////////////////////////////////////////////
// CxxType
////////////////////////////////////////////////////////////////////////////////

/** A C++ type.
 * Represents types associated with C++ language constructs.
 */
class CxxType {
 public:


    /** Create CxxType.
     * @param t CXType to represent.
     */
    CxxType(CXType t) : type_(t), canonical_type_(clang_getCanonicalType(t)) {
        name_ = to_string(clang_getTypeSpelling(type_));
        canonical_name_ = to_string(clang_getTypeSpelling(canonical_type_));
        is_pointer_ = (type_.kind == CXType_Pointer);
        is_lvalue_reference_ = (type_.kind == CXType_LValueReference);
        is_rvalue_reference_ = (type_.kind == CXType_RValueReference);
        is_const_ = clang_isConstQualifiedType(type_);
    }

  /** Infer type from cursor.
   * @param Clang AST cursor pointing to a typed C++ construct.
   */
    CxxType(CXCursor c) : CxxType(clang_getCursorType(c)) {}

  /// The type's name.
  std::string get_name() const { return name_; }

  /// The canonical name.
  std::string get_canonical_name() const { return canonical_name_; }

  /** Replace template variables in type names.
   *
   * Replace template variables in the types name and canonical name.
   *
   * @param template_names Vector containing the names of the template variables to replace.
   * @param template_arguments Vector containing the types and variables with which to
   *    replace the template variables.
   */
  void replace_template_variables(const std::vector<std::string> &template_names,
                                  const std::vector<std::string> &template_arguments) {
    name_ = pxx::ast::replace_template_variables(name_, template_names, template_arguments);
    canonical_name_ =
        pxx::ast::replace_template_variables(canonical_name_, template_names, template_arguments);
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
  bool is_eigen_type() const {
    const std::string &s = canonical_name_;
    return (s.find("Eigen::") != s.npos);
  }

  friend std::ostream &operator<<(std::ostream &stream, const CxxType &t);

 protected:
  std::string name_;
  std::string canonical_name_;
  CXCursor cursor_;
  CXType type_;
  CXType canonical_type_;
  bool is_const_;
  bool is_pointer_;
  bool is_lvalue_reference_;
  bool is_rvalue_reference_;
};

void to_json(json &j, const CxxType &t) {
    j["name"] = t.get_name();
    j["canonical_name"] = t.get_canonical_name();
}

std::ostream &operator<<(std::ostream &stream, const CxxType &t) {
  stream << t.get_name() << " (" << t.get_canonical_name()
         << ") ";
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Cxx Templates
////////////////////////////////////////////////////////////////////////////////

template<typename Base>
class CxxTemplateInstance : public Base
{
    using Base::replace_template_variables;
public:
    CxxTemplateInstance(std::string name_,
                        std::vector<std::string> template_arguments_,
                        std::vector<std::string> template_values_,
                        const Base &base)
        : Base(base), name(name_), template_values(template_values_) {
        replace_template_variables(template_arguments_, template_values_);
    }

    std::string get_name() const {
        std::stringstream name;
        name << Base::get_name();
        name << "<";
        for (size_t i = 0; i < template_values.size(); ++i) {
            name << template_values[i];
            if (i < template_values.size() - 1) {
                name << ", ";
            }
        }
        name << ">";
        return name.str();
    }

    std::string get_qualified_name() const {
        std::stringstream name;
        name << Base::get_qualified_name();
        name << "<";
        for (size_t i = 0; i < template_values.size(); ++i) {
            name << template_values[i];
            if (i < template_values.size() - 1) {
                name << ", ";
            }
        }
        name << ">";
        return name.str();
    }
    std::string name;
    std::vector<std::string> template_values;

};


template <typename Base>
    void to_json(json &j, const CxxTemplateInstance<Base> &t) {
    to_json(j, static_cast<Base>(t));
    j["name"] = t.get_name();
    j["qualified_name"] = t.get_qualified_name();
    j["export_name"] = t.name;
    j["template_values"] = t.template_values;

}

template <typename Base>
class CxxTemplate : public Base {

    using Base::cursor_;
    using Base::settings_;
    using Base::get_name;
    using Base::replace_template_variables;

public:

    ///template<typename... Ts>
    ///    CxxTemplate(Ts&&... args) : Base(std::forward<Ts>(args)...) {
    ///}
    CxxTemplate() = delete;

    CxxTemplate(CXCursor c, const CxxConstruct &p) : Base(c, p) {
        clang_visitChildren(cursor_, &parse_template<Base>, reinterpret_cast<CXClientData>(this));
    }

    void add_template_argument(CXCursor c) {
        template_arguments_.emplace_back(c, *this);
    }

    std::vector<CxxTemplateInstance<Base>> get_instances() const {

        std::vector<std::string> template_argument_names;
        for (auto &ta : template_arguments_) {
            template_argument_names.push_back(ta.get_name());
        }
        std::vector<CxxTemplateInstance<Base>> instances{};
        for (auto &is : settings_.instance_strings) {
            std::string export_name = std::get<0>(is);
            if (export_name == "") {
                export_name = get_name();
            }
            instances.emplace_back(export_name, template_argument_names, std::get<1>(is), *this);
        }
        return instances;
    }

    friend std::ostream &operator<<(std::ostream &stream, const CxxTemplate &f) {
        stream << "template<";
        for (size_t i = 0; i < f.template_arguments_.size(); ++i) {
            auto &p = f.template_arguments_[i];
            stream << p.get_name();
            if (i < f.template_arguments_.size() - 1) {
                stream << ", ";
            }
        }
        stream << "> " << static_cast<Base>(f);
        return stream;
    }


protected:

  std::vector<CxxConstruct> template_arguments_ = {};

};


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

class Variable : public CxxConstruct {
public:
  Variable(CXCursor c, const CxxConstruct &p) : CxxConstruct(c, p), type_(c) {}

  std::string get_typename() const { return type_.get_canonical_name(); }

  bool is_const() const { return type_.is_const(); }
  bool is_eigen_type() const { return type_.is_eigen_type(); }
  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const Variable &v);

  void replace_template_variables(const std::vector<std::string> &template_names,
                                  const std::vector<std::string> &template_arguments) {
        type_.replace_template_variables(template_names, template_arguments);
    }

protected:
  std::string typename_;
  CxxType type_;
};

void to_json(json &j, const Variable &v) {
  j["name"] = v.name_;
  j["typename"] = v.get_typename();
  j["const"] = v.type_.is_const();
}

std::ostream &operator<<(std::ostream &stream, const Variable &v) {
  stream << v.typename_ << " " << v.name_ << std::endl;
  return stream;
}

class MemberVariable : public Variable {
public:
  MemberVariable(CXCursor c, const CxxConstruct &p) : Variable(c, p) {

    // retrieve access specifier.
    CX_CXXAccessSpecifier as = clang_getCXXAccessSpecifier(c);
    if (as == CX_CXXPublic) {
      access_specifier_ = access_specifier_t::pub;
    } else if (as == CX_CXXProtected) {
      access_specifier_ = access_specifier_t::prot;
    } else if (as == CX_CXXPrivate) {
      access_specifier_ = access_specifier_t::priv;
    } else {
      valid_ = false;
    }

    // retrieve storage class.
    CX_StorageClass sc = clang_Cursor_getStorageClass(c);
    if (sc == CX_SC_None) {
      storage_class_ = storage_class_t::none;
    } else if (sc == CX_SC_Extern) {
      storage_class_ = storage_class_t::ext;
    } else if (sc == CX_SC_Static) {
      storage_class_ = storage_class_t::stat;
    } else {
      valid_ = false;
    }
  }

  std::string get_type_spelling() const { return typename_; }

  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const MemberVariable &v);

private:
  access_specifier_t access_specifier_;
  storage_class_t storage_class_;
};

void to_json(json &j, const MemberVariable &v) {
  j["name"] = v.name_;
  j["qualified_name"] = v.get_qualified_name();
  j["typename"] = v.typename_;
  j["access"] = to_string(v.access_specifier_);
  j["storage"] = to_string(v.storage_class_);
  j["const"] = v.is_const();
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

using pxx::print_children;
using pxx::operator<<;

class Function : public CxxConstruct {
public:
  Function(CXCursor c, const CxxConstruct &p)
      : CxxConstruct(c, p),
        result_type_{clang_getResultType(clang_getCursorType(c))}
  {
      clang_visitChildren(cursor_, &parse_function,
                          reinterpret_cast<CXClientData>(this));
  }

  void add_parameter(CXCursor c) {
    parameters_.emplace_back(c, *this);
  }


  bool uses_eigen() const {
      bool eigen_found = result_type_.is_eigen_type();
      eigen_found |= std::any_of(parameters_.begin(),
                                 parameters_.end(),
                                 [](const Variable &p){ return p.is_eigen_type(); });
      return eigen_found;
  }

  void replace_template_variables(const std::vector<std::string> &template_names,
                                  const std::vector<std::string> &template_arguments) {
      for (auto &p : parameters_) {
          p.replace_template_variables(template_names, template_arguments);
      }
      result_type_.replace_template_variables(template_names, template_arguments);
  }

  friend std::ostream &operator<<(std::ostream &stream, const Function &cl);
  friend void to_json(json &j, const Function &f);

protected:
  std::vector<Variable> parameters_;
  CxxType result_type_;
};

using FunctionTemplate = CxxTemplate<Function>;
using FunctionTemplateInstance = CxxTemplateInstance<Function>;

void to_json(json &j, const Function &f) {
  j["name"] = f.get_name();
  j["qualified_name"] = f.get_qualified_name();
  j["parameters"] = f.parameters_;
  j["result_type"] = f.result_type_;

}

std::ostream &operator<<(std::ostream &stream, const Function &f) {
  stream << f.name_ << "(";
  for (size_t i = 0; i < f.parameters_.size(); ++i) {
    auto &p = f.parameters_[i];
    stream << p.get_typename();
    if (i < f.parameters_.size() - 1) {
      stream << ", ";
    }
  }
  stream << ") exported = "  << f.get_export_settings().exp << std::endl;
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Function Template
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class Class : public CxxConstruct {
public:
  Class(CXCursor c, const CxxConstruct &p) : CxxConstruct(c, p) {
    // Continue AST traversal.
    clang_visitChildren(cursor_, &parse_class,
                        reinterpret_cast<CXClientData>(this));
  }

  void add_constructor(CXCursor c) { constructors_.emplace_back(c, *this); }
  void add_method(CXCursor c) { methods_.emplace_back(c, *this); }
  void add_data_member(CXCursor c) { data_members_.emplace_back(c, *this); }

  friend std::ostream &operator<<(std::ostream &stream, const Class &cl);
  friend void to_json(json &j, const Class &c);

  bool uses_eigen() const {
      bool eigen_found = false;
      eigen_found |= std::any_of(constructors_.begin(),
                                 constructors_.end(),
                                 [](const Function &p){ return p.uses_eigen(); });
      eigen_found |= std::any_of(methods_.begin(),
                                 methods_.end(),
                                 [](const Function &p){ return p.uses_eigen(); });
      eigen_found |= std::any_of(data_members_.begin(),
                                 data_members_.end(),
                                 [](const Variable &p){ return p.is_eigen_type(); });
      return eigen_found;
  }

  void replace_template_variables(
      const std::vector<std::string> &template_names,
      const std::vector<std::string> &template_arguments) {
    for (auto &c : constructors_) {
      c.replace_template_variables(template_names, template_arguments);
    }
    for (auto &m : methods_) {
      m.replace_template_variables(template_names, template_arguments);
    }
    for (auto &d : data_members_) {
      d.replace_template_variables(template_names, template_arguments);
    }
  }

  std::vector<Function> get_exported_methods() const {
      std::vector<Function> exports;
      std::copy_if (methods_.begin(),
                    methods_.end(),
                    std::back_inserter(exports),
                    [](const Function &f){return f.get_export_settings().exp;});
      return exports;
  }

  std::vector<Function> get_exported_constructors() const {
      std::vector<Function> exports;
      std::copy_if (constructors_.begin(),
                    constructors_.end(),
                    std::back_inserter(exports),
                    [](const Function &f){return f.get_export_settings().exp;});
      return exports;
  }

  std::vector<MemberVariable> get_exported_variables() const {
      std::vector<MemberVariable> exports;
      std::copy_if (data_members_.begin(),
                    data_members_.end(),
                    std::back_inserter(exports),
                    [](const MemberVariable &v){return v.get_export_settings().exp;});
      return exports;
  }

private:
  std::vector<Function> constructors_;
  std::vector<Function> methods_;
  std::vector<MemberVariable> data_members_;
};

using ClassTemplate = CxxTemplate<Class>;
using ClassTemplateInstance = CxxTemplateInstance<Class>;

void to_json(json &j, const Class &c) {
  j["name"] = c.get_name();
  j["qualified_name"] = c.get_qualified_name();
  j["constructors"] = c.constructors_;
  j["methods"] = c.methods_;
  j["data_members"] = c.data_members_;
}

std::ostream &operator<<(std::ostream &stream, const Class &cl) {
  stream << "Class: " << cl.name_ << std::endl;
  stream << "Constructors:" << std::endl;
  for (auto &&f : cl.constructors_) {
    stream << "\t" << f;
  }
  stream << "Methods:" << std::endl;
  for (auto &&f : cl.methods_) {
    stream << "\t" << f;
  }
  stream << "Data members:" << std::endl;
  for (auto &&v : cl.data_members_) {
    stream << "\t" << v;
  }

  stream << "\t" << "Exported: " << cl.settings_.exp << std::endl;
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////////////

class Namespace : public CxxConstruct {
public:
Namespace(CXCursor c, ExportSettings s = ExportSettings()) : CxxConstruct(c, s) {
    clang_visitChildren(cursor_, &parse_namespace,
                        reinterpret_cast<CXClientData>(this));
  }

  Namespace(CXCursor c, const CxxConstruct &p) : CxxConstruct(c, p) {
    clang_visitChildren(cursor_, &parse_namespace,
                        reinterpret_cast<CXClientData>(this));
  }

  void add_class(CXCursor c) { classes_.emplace_back(c, *this); }
  void add_class_template(CXCursor c) { class_templates_.emplace_back(c, *this); }
  void add_function(CXCursor c) { functions_.emplace_back(c, *this); }
  void add_function_template(CXCursor c) { function_templates_.emplace_back(c, *this); }
  void add_namespace(CXCursor c) {
    std::string name = pxx::ast::get_name(c);
    auto it = namespaces_.find(name);
    if (it != namespaces_.end()) {
      clang_visitChildren(c, &parse_namespace,
                          reinterpret_cast<CXClientData>(&(it->second)));
    } else {
      namespaces_.insert(std::make_pair(name, Namespace(c, *this)));
    }
  }

  bool uses_eigen() const {
      std::cout << "ns name: " << name_ << std::endl;
      if (name_.find("Eigen") != std::string::npos) {
          return true;
      }
      bool eigen_found = false;
      eigen_found |= std::any_of(namespaces_.begin(),
                                 namespaces_.end(),
                                 [](const auto &n){ return n.second.uses_eigen(); });
      eigen_found |= std::any_of(classes_.begin(),
                                 classes_.end(),
                                 [](const Class &c){ return c.uses_eigen(); });
      eigen_found |= std::any_of(functions_.begin(),
                                 functions_.end(),
                                 [](const Function &f){ return f.uses_eigen(); });
      return eigen_found;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Namespace &ns);
  friend void to_json(json &j, const Namespace &ns);

  void print() { std::cout << *this << std::endl; }

  std::vector<Function> get_exported_functions() const {
      std::vector<Function> exports;
      std::copy_if (functions_.begin(),
                    functions_.end(),
                    std::back_inserter(exports),
                    [](const Function &f){return f.get_export_settings().exp;});
      for(auto &&n : namespaces_) {
          auto nested_functions = n.second.get_exported_functions();
          exports.insert(exports.end(), nested_functions.begin(), nested_functions.end());
      }
      return exports;
  }

  std::vector<Class> get_exported_classes() const {
      std::vector<Class> exports;
      std::copy_if (classes_.begin(),
                    classes_.end(),
                    std::back_inserter(exports),
                    [](const Class &c){return c.get_export_settings().exp;});
      for(auto &&n : namespaces_) {
          auto nested_classes = n.second.get_exported_classes();
          exports.insert(exports.end(), nested_classes.begin(), nested_classes.end());
      }
      return exports;
  }

  std::vector<FunctionTemplateInstance> get_function_template_instances() const {
      std::vector<FunctionTemplateInstance> exports;
      for(auto &&ft : function_templates_) {
            auto instances = ft.get_instances();
            exports.insert(exports.end(), instances.begin(), instances.end());
      }
      for(auto &&n : namespaces_) {
          auto nested_instances = n.second.get_function_template_instances();
          exports.insert(exports.end(), nested_instances.begin(), nested_instances.end());
      }
      return exports;
  }

  std::vector<ClassTemplateInstance> get_class_template_instances() const {
      std::vector<ClassTemplateInstance> exports;
      for(auto &&ft : class_templates_) {
          auto instances = ft.get_instances();
          exports.insert(exports.end(), instances.begin(), instances.end());
      }
      for(auto &&n : namespaces_) {
          auto nested_instances = n.second.get_class_template_instances();
          exports.insert(exports.end(), nested_instances.begin(), nested_instances.end());
      }
      return exports;
  }

protected:
  std::vector<Class> classes_;
  std::vector<ClassTemplate> class_templates_;
  std::vector<Function> functions_;
  std::vector<FunctionTemplate> function_templates_;
  std::map<std::string, Namespace> namespaces_;
};

void to_json(json &j, const Namespace &ns) {
  auto classes = ns.get_exported_classes();
  auto functions = ns.get_exported_functions();
  auto function_template_instances = ns.get_function_template_instances();
  auto class_template_instances = ns.get_class_template_instances();

  j["classes"] = classes;
  j["functions"] = functions;
  j["function_template_instances"] = function_template_instances;
  j["class_template_instances"] = class_template_instances;
}

std::ostream &operator<<(std::ostream &stream, const Namespace &ns) {
  stream << "A C++ namespace: " << ns.get_qualified_name() << std::endl << std::endl;
  stream << "Defined classes:" << std::endl;
  for (auto &&cl : ns.get_exported_classes()) {
    std::cout << std::endl;
    std::cout << cl;
  }
  stream << "Defined functions:" << std::endl;
  for (auto &&f : ns.get_exported_functions()) {
      std::cout << std::endl;
      std::cout << f;
  }
  stream << "Class templates" << std::endl;
  for (auto &&c : ns.class_templates_) {
      std::cout << std::endl;
      std::cout << c;
  }

  stream << "Function templates" << std::endl;
  for (auto &&f : ns.function_templates_) {
      std::cout << std::endl;
      std::cout << f;
  }
  return stream;
}

class TranslationUnit : public Namespace {
public:
  TranslationUnit(CXCursor c, ExportSettings s = ExportSettings())
      : Namespace(c, s) {}
  friend std::ostream &operator<<(std::ostream &stream,
                                  const TranslationUnit &tu);

  bool has_standard_headers() {
    return (namespaces_.find("std") != namespaces_.end());
  }

  bool has_eigen_headers() {
    return (namespaces_.find("Eigen") != namespaces_.end());
  }

  void dump_ast() {
      detail::AstFormatter ast_formatter{};
      clang_visitChildren(cursor_, detail::AstFormatter::traverse, &ast_formatter);
  }
};

std::ostream &operator<<(std::ostream &stream, const TranslationUnit &tu) {
  stream << "A C++ translation unit:" << tu.name_ << std::endl << std::endl;
  stream << "Defined functions:" << std::endl;
  for (auto &&f : tu.get_exported_functions()) {
      std::cout << std::endl;
      std::cout << f;
  }
  stream << "Defined classes:" << std::endl;
  for (auto &&cl : tu.get_exported_classes()) {
    std::cout << std::endl;
    std::cout << cl;
  }
  stream << "Defined namespaces:" << std::endl;
  for (auto &&n : tu.namespaces_) {
      std::cout << std::endl;
      std::cout << n.second;
  }
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Translation Unit
////////////////////////////////////////////////////////////////////////////////

CXChildVisitResult parse_namespace(CXCursor c,
                                   CXCursor /*parent*/,
                                   CXClientData client_data) {
  Namespace *ns = reinterpret_cast<Namespace *>(client_data);
  CXCursorKind k = clang_getCursorKind(c);
  switch (k) {
  case CXCursor_ClassDecl: {
    ns->add_class(c);
  } break;
  case CXCursor_FunctionDecl: {
    ns->add_function(c);
  } break;
  case CXCursor_FunctionTemplate: {
      ns->add_function_template(c);
  } break;
  case CXCursor_ClassTemplate: {
      ns->add_class_template(c);
  } break;
  case CXCursor_Namespace: {
    ns->add_namespace(c);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult parse_class(CXCursor c, CXCursor /*parent*/,
                               CXClientData client_data) {
  Class *cl = reinterpret_cast<Class *>(client_data);
  CXCursorKind k = clang_getCursorKind(c);
  switch (k) {
  case CXCursor_Constructor: {
    cl->add_constructor(c);
  } break;
  case CXCursor_CXXMethod: {
    cl->add_method(c);
  } break;
  case CXCursor_FieldDecl: {
    cl->add_data_member(c);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult parse_function(CXCursor c, CXCursor /*parent*/,
                                  CXClientData client_data) {
  Function *f = reinterpret_cast<Function *>(client_data);
  CXCursorKind k = clang_getCursorKind(c);
  switch (k) {
  case CXCursor_ParmDecl: {
    f->add_parameter(c);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}

template<typename Base>
CXChildVisitResult parse_template(CXCursor c,
                                  CXCursor /*parent*/,
                                  CXClientData client_data) {
    CxxTemplate<Base> *ft = reinterpret_cast<CxxTemplate<Base> *>(client_data);
    CXCursorKind k = clang_getCursorKind(c);
    switch (k) {
    case CXCursor_TemplateTypeParameter: {
        ft->add_template_argument(c);
    } break;
    case CXCursor_NonTypeTemplateParameter: {
        ft->add_template_argument(c);
    } break;
    default:
        break;
    }
    return CXChildVisit_Continue;
}

} // namespace pxx::ast

#endif
