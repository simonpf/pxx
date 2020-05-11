#ifndef __PXX_AST_H__
#define __PXX_AST_H__

#include <clang-c/Index.h>
#include <inja/inja.hpp>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace pxx::ast {

CXChildVisitResult parse_function(CXCursor c, CXCursor parent,
                                  CXClientData client_data);

CXChildVisitResult parse_class(CXCursor c, CXCursor parent,
                               CXClientData client_data);

CXChildVisitResult parse_translation_unit(CXCursor c, CXCursor parent,
                                          CXClientData client_data);

////////////////////////////////////////////////////////////////////////////////
// CxxObject
////////////////////////////////////////////////////////////////////////////////

class CxxObject {
public:
  CxxObject(CXCursor c) : cursor_(c) {

    CXString s = clang_getCursorSpelling(c);
    name_ = clang_getCString(s);
    clang_disposeString(s);
  }

  std::string get_name() { return name_; }

  friend std::ostream &operator<<(std::ostream &stream, const CxxObject &cl);

protected:
  CXCursor cursor_;
  std::string name_;
};

////////////////////////////////////////////////////////////////////////////////
// Variable
////////////////////////////////////////////////////////////////////////////////

class Variable : public CxxObject {
public:
  Variable(CXCursor c) : CxxObject(c) {
    type_ = clang_getCursorType(c);
    CXString s = clang_getTypeSpelling(type_);
    type_spelling_ = clang_getCString(s);
    clang_disposeString(s);
  }

  std::string get_type_spelling() const { return type_spelling_; }
  friend std::ostream &operator<<(std::ostream &stream, const Variable &cl);
  friend void to_json(json &j, const Variable &v);

private:
  std::string type_spelling_;
  CXType type_;
};

void to_json(json &j, const Variable &v) {
  j["name"] = v.name_;
  j["type_spelling"] = v.type_spelling_;
}

std::ostream &operator<<(std::ostream &stream, const Variable &v) {
  stream << v.type_spelling_ << " " << v.name_ << std::endl;
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

class Function : public CxxObject {
public:
  Function(CXCursor c) : CxxObject(c) {
    // Continue AST traversal.
    clang_visitChildren(cursor_, &parse_function,

                        reinterpret_cast<CXClientData>(this));
  }
  void add_parameter(Variable v) { parameters_.push_back(v); }
  friend std::ostream &operator<<(std::ostream &stream, const Function &cl);
  friend void to_json(json &j, const Function &f);

private:
  std::vector<Variable> parameters_;
};

void to_json(json &j, const Function &f) {
  j["name"] = f.name_;
  j["cxx_spelling"] = f.name_;
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
  Class(CXCursor c) : CxxObject(c) {
    // Continue AST traversal.
    clang_visitChildren(cursor_, &parse_class,
                        reinterpret_cast<CXClientData>(this));
  }

  void add_constructor(Function f) { constructors_.push_back(f); }

  void add_method(Function f) { methods_.push_back(f); }

  void add_data_member(Variable v) { data_members_.push_back(v); }

  friend std::ostream &operator<<(std::ostream &stream, const Class &cl);
  friend void to_json(json &j, const Class &c);

private:
  std::vector<Function> constructors_;
  std::vector<Function> methods_;
  std::vector<Variable> data_members_;
};

void to_json(json &j, const Class &c) {
  j["name"] = c.name_;
  j["cxx_spelling"] = c.name_;
  j["python_spelling"] = c.name_;
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
// Translation Unit
////////////////////////////////////////////////////////////////////////////////

class TranslationUnit {
public:
  TranslationUnit(CXCursor c) : cursor_(c) {
    clang_visitChildren(cursor_, &parse_translation_unit,
                        reinterpret_cast<CXClientData>(this));
  }
  void add(const Class &cl) { classes_.push_back(cl); }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const TranslationUnit &tu);
  friend void to_json(json &j, const TranslationUnit &tu);

  void print() { std::cout << *this << std::endl; }

  void to_json() {
    json value{};
    value["classes"] = classes_;
    std::cout << value << std::endl;
  }

private:
  CXCursor cursor_;
  std::vector<Class> classes_;
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

void to_json(json &j, const TranslationUnit &tu) { j["classes"] = tu.classes_; }

////////////////////////////////////////////////////////////////////////////////
// Translation Unit
////////////////////////////////////////////////////////////////////////////////

CXChildVisitResult parse_translation_unit(CXCursor c, CXCursor parent,
                                          CXClientData client_data) {
  TranslationUnit *tu = reinterpret_cast<TranslationUnit *>(client_data);
  CXCursorKind k = clang_getCursorKind(c);
  switch (k) {
  case CXCursor_ClassDecl: {
    Class cl(c);
    tu->add(cl);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult parse_class(CXCursor c, CXCursor parent,
                               CXClientData client_data) {
  Class *cl = reinterpret_cast<Class *>(client_data);
  CXCursorKind k = clang_getCursorKind(c);
  switch (k) {
  case CXCursor_Constructor: {
    Function f(c);
    cl->add_constructor(f);
  } break;
  case CXCursor_CXXMethod: {
    Function f(c);
    cl->add_method(f);
  } break;
  case CXCursor_FieldDecl: {
    Variable v(c);
    cl->add_data_member(v);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult parse_function(CXCursor c, CXCursor parent,
                                  CXClientData client_data) {
  Function *f = reinterpret_cast<Function *>(client_data);
  CXCursorKind k = clang_getCursorKind(c);
  switch (k) {
  case CXCursor_ParmDecl: {
    Variable p(c);
    f->add_parameter(p);
  } break;
  default:
    break;
  }
  return CXChildVisit_Continue;
}
} // namespace pxx::ast

#endif
