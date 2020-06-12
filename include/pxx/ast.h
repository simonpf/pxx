#ifndef __PXX_AST_H__
#define __PXX_AST_H__

#include <clang-c/Index.h>
#include <inja/inja.hpp>
#include <iostream>
#include <pxx/utils.h>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace pxx::ast {

using pxx::to_string;

CXChildVisitResult parse_function(CXCursor c, CXCursor parent,
                                  CXClientData client_data);

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

////////////////////////////////////////////////////////////////////////////////
// CxxConstruct
////////////////////////////////////////////////////////////////////////////////

class CxxConstruct {
public:
  CxxConstruct(CXCursor c) : cursor_(c), name_(pxx::ast::get_name(c)) {}

  CxxConstruct(CXCursor c, const CxxConstruct &p) : cursor_(c) {
    CXString s = clang_getCursorSpelling(c);
    name_ = to_string(s);
    std::string parent_name = p.get_qualified_name();
    if (parent_name != "") {
      qualified_name_ = p.get_qualified_name() + "::" + name_;
    } else {
      qualified_name_ = name_;
    }
  }

  std::string get_name() const { return name_; }
  std::string get_qualified_name() const { return qualified_name_; }

  friend std::ostream &operator<<(std::ostream &stream, const CxxConstruct &cl);

protected:
  CXCursor cursor_;
  std::string name_;
  std::string qualified_name_;
  bool valid_ = true;
};

class CxxType {
public:

  CxxType(CXCursor c)
      : cursor_(c),
        type_(clang_getCursorType(c)),
        canonical_type_(clang_getCanonicalType(clang_getCursorType(c)))

  {
    is_pointer_ = (type_.kind == CXType_Pointer);
    is_lvalue_reference_ = (type_.kind == CXType_LValueReference);
    is_rvalue_reference_ = (type_.kind == CXType_RValueReference);
    is_const_ = clang_isConstQualifiedType(type_);
  }

  CxxType(CXType t)
      : type_(t),
        canonical_type_(clang_getCanonicalType(t)) {}

  std::string get_type_spelling() const {
    return to_string(clang_getTypeSpelling(type_));
  }

  std::string get_canonical_type_spelling() const {
      return to_string(clang_getTypeSpelling(type_));
  }

  bool is_const() const { return is_const_; }
  bool is_pointer() const { return is_pointer_; }
  bool is_lvalue_reference() const { return is_lvalue_reference_; }
  bool is_rvalue_reference() const { return is_rvalue_reference_; }

  bool is_eigen_type() const {
      std::string s = get_canonical_type_spelling();
      std::cout << "type: " << s << std::endl;
      return (s.find("Eigen::") != s.npos);
  }

  friend std::ostream &operator<<(std::ostream &stream, const CxxType &t);

protected:
  CXCursor cursor_;
  CXType type_;
  CXType canonical_type_;
  bool is_const_;
  bool is_pointer_;
  bool is_lvalue_reference_;
  bool is_rvalue_reference_;
};

std::ostream &operator<<(std::ostream &stream, const CxxType &t) {
  stream << t.get_type_spelling() << " (" << t.get_canonical_type_spelling()
         << ") ";
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

class Variable : public CxxConstruct {
public:
  Variable(CXCursor c, const CxxConstruct &p) : CxxConstruct(c, p), type_(c) {}

  std::string get_type_spelling() const { return type_.get_type_spelling(); }

  bool is_const() const { return type_.is_const(); }
  bool is_eigen_type() const { return type_.is_eigen_type(); }
  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const Variable &v);

protected:
  std::string type_name_;
  CxxType type_;
};

void to_json(json &j, const Variable &v) {
  j["name"] = v.name_;
  j["type_name"] = v.type_.get_type_spelling();
  j["const"] = v.type_.is_const();
}

std::ostream &operator<<(std::ostream &stream, const Variable &v) {
  stream << v.type_name_ << " " << v.name_ << std::endl;
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

  std::string get_type_spelling() const { return type_name_; }

  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const MemberVariable &v);

private:
  access_specifier_t access_specifier_;
  storage_class_t storage_class_;
};

void to_json(json &j, const MemberVariable &v) {
  j["name"] = v.name_;
  j["qualified_name"] = v.get_qualified_name();
  j["type_name"] = v.type_name_;
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
        result_type_(clang_getResultType(clang_getCursorType(c)))
  {
    // Parse parameters.
    size_t n = clang_Cursor_getNumArguments(c);
    for (size_t i = 0; i < n; ++i) {
      add_parameter(clang_Cursor_getArgument(c, i));
    }
  }
  void add_parameter(CXCursor c) {
    using pxx::operator<<;

    parameters_.emplace_back(c, *this);
  }

  bool uses_eigen() const {
      bool eigen_found = result_type_.is_eigen_type();
      eigen_found |= std::any_of(parameters_.begin(),
                                 parameters_.end(),
                                 [](const Variable &p){ return p.is_eigen_type(); });
      return eigen_found;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Function &cl);
  friend void to_json(json &j, const Function &f);

private:
  std::vector<Variable> parameters_;
  CxxType result_type_;
};

void to_json(json &j, const Function &f) {
  j["name"] = f.get_name();
  j["qualified_name"] = f.get_qualified_name();
  j["parameters"] = f.parameters_;
}

std::ostream &operator<<(std::ostream &stream, const Function &f) {
  stream << f.name_ << "(";
  for (size_t i = 0; i < f.parameters_.size(); ++i) {
    auto &p = f.parameters_[i];
    stream << p.get_type_spelling();
    if (i < f.parameters_.size() - 1) {
      stream << ", ";
    }
  }
  stream << ")" << std::endl;
  return stream;
}

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

private:
  std::vector<Function> constructors_;
  std::vector<Function> methods_;
  std::vector<MemberVariable> data_members_;
};

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
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////////////

class Namespace : public CxxConstruct {
public:
  Namespace(CXCursor c) : CxxConstruct(c) {
    clang_visitChildren(cursor_, &parse_namespace,
                        reinterpret_cast<CXClientData>(this));
  }

  Namespace(CXCursor c, const CxxConstruct &p) : CxxConstruct(c, p) {
    clang_visitChildren(cursor_, &parse_namespace,
                        reinterpret_cast<CXClientData>(this));
  }

  void add_class(CXCursor c) { classes_.emplace_back(c, *this); }
  void add_function(CXCursor c) { functions_.emplace_back(c, *this); }
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
      bool eigen_found = false;
      eigen_found |= std::any_of(classes_.begin(),
                                 classes_.end(),
                                 [](const Class &c){ return c.uses_eigen(); });
      eigen_found |= std::any_of(functions_.begin(),
                                 functions_.end(),
                                 [](const Function &f){ return f.uses_eigen(); });
      eigen_found |= std::any_of(namespaces_.begin(),
                                 namespaces_.end(),
                                 [](const auto &n){ return n.second.uses_eigen(); });
      return eigen_found;
  }

  friend std::ostream &operator<<(std::ostream &stream, const Namespace &ns);
  friend void to_json(json &j, const Namespace &ns);

  void print() { std::cout << *this << std::endl; }

protected:
  std::vector<Class> classes_;
  std::vector<Function> functions_;
  std::map<std::string, Namespace> namespaces_;
};

void to_json(json &j, const Namespace &ns) {
  j["classes"] = ns.classes_;
  j["functions"] = ns.functions_;
  j["namespaces"] = ns.namespaces_;
}

std::ostream &operator<<(std::ostream &stream, const Namespace &ns) {
  stream << "A C++ namespace unit." << std::endl << std::endl;
  stream << "Defined classes:" << std::endl;
  for (auto &&cl : ns.classes_) {
    std::cout << std::endl;
    std::cout << cl;
  }
  return stream;
}

class TranslationUnit : public Namespace {
public:
  TranslationUnit(CXCursor c) : Namespace(c) {}
  friend std::ostream &operator<<(std::ostream &stream,
                                  const TranslationUnit &tu);

  bool has_standard_headers() {
    return (namespaces_.find("std") != namespaces_.end());
  }

  bool has_eigen_headers() {
    return (namespaces_.find("Eigen") != namespaces_.end());
  }
};

std::ostream &operator<<(std::ostream &stream, const TranslationUnit &tu) {
  stream << "A C++ translation unit." << std::endl << std::endl;
  stream << "Defined classes:" << std::endl;
  for (auto &&cl : tu.classes_) {
    std::cout << std::endl;
    std::cout << cl;
  }
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Translation Unit
////////////////////////////////////////////////////////////////////////////////

CXChildVisitResult parse_namespace(CXCursor c, CXCursor /*parent*/,
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
} // namespace pxx::ast

#endif
