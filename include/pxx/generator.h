/** \file pxx/generator.h
 *
 * This file defines the Generator class which provides the top-level interface
 * to the functionality of pxx.
 *
 */
#ifndef __PXX_GENERATOR_H__
#define __PXX_GENERATOR_H__

#include <string>
#include <memory>
#include <fstream>
#include <sstream>

#include <clang-c/Index.h>
#include <pxx/cxx/parser.h>

namespace pxx {

class Module {

  void write_module_header(std::ostream &output) {
    output << R"(
#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>

namespace py = pybind11;
        )";
  }

public:
  Module(cxx::ASTNode *ast,
        cxx::Scope *scope,
        std::string name,
         std::string output_file = "")
      : ast_(ast), scope_(scope), name_(name), output_file_(output_file) {}

  void write_bindings() {
    std::ostream *output = &std::cout;
    std::unique_ptr<std::ostream> output_file = nullptr;
    if (output_file_.size() != 0) {
      output_file = std::make_unique<std::ofstream>(output_file_);
      output = output_file.get();
    }
    write_module_header(*output);
    ast_->write_bindings(*output);
  }

  std::string print_bindings() {
    std::stringstream output{};

    write_module_header(output);
    ast_->write_bindings(output);
    return output.str();
  }

private:
  cxx::ASTNode *ast_;
  cxx::Scope *scope_;
  std::string name_;
  std::string output_file_;
};

  /** Generator for Python interfaces.
   *
   * This class is responsible of generating Python bindings
   * from a C++ translation unit.
   */
  class Generator {

  public:
    /** Create a new generator for a given C++ source file.
     *
     * @param filename Path to a C++ file for which to generate Python
     * bindings.
     * @param extra_arguments Vector of strings containing extra arguments
     * that may be required to parse the translation unit and which will be
     * passed to clang.
     */
    Generator(std::string filename, std::vector<std::string> extra_arguments)
        : filename_(filename), extra_arguments_(extra_arguments) {
      cxx::Parser parser(filename, extra_arguments);
      auto results = parser.parse();
      ast_ = std::unique_ptr<cxx::ASTNode>(std::get<0>(results));
      scope_ = std::unique_ptr<cxx::Scope>(std::get<1>(results));
    }


    std::string print_bindings() {
        Module module(ast_.get(), scope_.get(), "", "");
        return module.print_bindings();

    }

  void print_ast() { ast_->print_tree(std::cout, 2); }

private:
  std::string filename_;
  std::vector<std::string> extra_arguments_;

  std::unique_ptr<cxx::ASTNode> ast_;
  std::unique_ptr<cxx::Scope> scope_;
};

} // namespace pxx
#endif
