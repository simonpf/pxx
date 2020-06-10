#ifndef __PXX_AST_H__
#define __PXX_AST_H__

#include <clang-c/Index.h>
#include <inja/inja.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <pxx/utils.h>

using json = nlohmann::json;

namespace pxx::ast {

CXChildVisitResult parse_function(CXCursor c, CXCursor parent,
                                  CXClientData client_data);

CXChildVisitResult parse_class(CXCursor c, CXCursor parent,
                               CXClientData client_data);

CXChildVisitResult parse_namespace(CXCursor c, CXCursor parent,
                                   CXClientData client_data);

enum class access_specifier_t {pub, prot, priv};
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

enum class storage_class_t {ext, stat, none};
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
    std::string name = clang_getCString(s);
    clang_disposeString(s);
    return name;
}

////////////////////////////////////////////////////////////////////////////////
// CxxObject
////////////////////////////////////////////////////////////////////////////////

class CxxObject {
public:
CxxObject(CXCursor c) : cursor_(c), name_(pxx::ast::get_name(c)) {}

CxxObject(CXCursor c, const CxxObject &p) : cursor_(c) {
    CXString s = clang_getCursorSpelling(c);
    name_ = clang_getCString(s);
    std::string parent_name = p.get_qualified_name();
    if (parent_name != "") {
      qualified_name_ = p.get_qualified_name() + "::" + name_;
    } else {
      qualified_name_ = name_;
    }
    clang_disposeString(s);
  }

    std::string get_name() const { return name_; }
  std::string get_qualified_name() const { return qualified_name_; }

  friend std::ostream &operator<<(std::ostream &stream, const CxxObject &cl);

protected:
  CXCursor cursor_;
  std::string name_;
  std::string qualified_name_;
  bool valid_ = true;
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

class Variable : public CxxObject {
public:
  Variable(CXCursor c, const CxxObject &p) : CxxObject(c, p) {
    type_ = clang_getCursorType(c);
    CXString s = clang_getTypeSpelling(type_);
    type_name_ = clang_getCString(s);
    clang_disposeString(s);
    const_ = static_cast<bool>(clang_isConstQualifiedType(type_));
  }

  std::string get_type_spelling() const { return type_name_; }
  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const Variable &v);

protected:
  std::string type_name_;
  bool const_;
  CXType type_;
};

void to_json(json &j, const Variable &v) {
  j["name"] = v.name_;
  j["type_name"] = v.type_name_;
  j["const"] = v.const_;
}

std::ostream &operator<<(std::ostream &stream, const Variable &v) {
  stream << v.type_name_ << " " << v.name_ << std::endl;
  return stream;
}

class MemberVariable : public Variable {
public:
  MemberVariable(CXCursor c, const CxxObject &p) : Variable(c, p) {

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

std::string
get_type_spelling() const {
  return type_name_;
}

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
    j["const"] = v.const_;
}


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

class Function : public CxxObject {
public:
  Function(CXCursor c, const CxxObject &p) : CxxObject(c, p) {
    // Continue AST traversal.
    clang_visitChildren(cursor_, &parse_function,

                        reinterpret_cast<CXClientData>(this));
  }
  void add_parameter(CXCursor c) { parameters_.emplace_back(c, *this); }
  friend std::ostream &operator<<(std::ostream &stream, const Function &cl);
  friend void to_json(json &j, const Function &f);

private:
  std::vector<Variable> parameters_;
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

class Class : public CxxObject {
public:
  Class(CXCursor c, const CxxObject &p) : CxxObject(c, p) {
    // Continue AST traversal.
    clang_visitChildren(cursor_, &parse_class,
                        reinterpret_cast<CXClientData>(this));
  }

  void add_constructor(CXCursor c) { constructors_.emplace_back(c, *this); }
  void add_method(CXCursor c) { methods_.emplace_back(c, *this); }
  void add_data_member(CXCursor c) { data_members_.emplace_back(c, *this); }

  friend std::ostream &operator<<(std::ostream &stream, const Class &cl);
  friend void to_json(json &j, const Class &c);

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

class Namespace : public CxxObject {
public:

  Namespace(CXCursor c) : CxxObject(c) {
    clang_visitChildren(cursor_, &parse_namespace,
                        reinterpret_cast<CXClientData>(this));
  }

  Namespace(CXCursor c, const CxxObject &p) : CxxObject(c, p) {
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
  friend std::ostream &operator<<(std::ostream &stream, const TranslationUnit &tu);

  bool has_standard_headers() {
      return (namespaces_.find("std") != namespaces_.end());
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
  case CXCursor_Namespace: {
      ns->add_namespace(c);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult parse_class(CXCursor c,
                               CXCursor /*parent*/,
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

CXChildVisitResult parse_function(CXCursor c,
                                  CXCursor /*parent*/,
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

