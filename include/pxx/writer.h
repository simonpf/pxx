/** \file pxx/writer.h
 *
 * This file defines the writer class which takes a pxx AST and writes
 * the corresponding pybind11 interface file.
 *
 */
#ifndef __PXX_WRITER_H__
#define __PXX_WRITER_H__

#include<iostream>
#include<string>
#include<vector>

#include <pxx/cxx/scope.h>
#include <pxx/cxx/ast.h>
#include <pxx/settings.h>

namespace pxx {

namespace detail {


void write_file_header(std::ostream &output,
                       const cxx::Scope* scope,
                       const Settings &settings) {

    if (settings.header == "") {
      output << std::endl;
      output << "////////////////////////////////////////////" << std::endl;
      output << "// Python bindings auto-generated by pxx. //" << std::endl;
      output << "////////////////////////////////////////////" << std::endl;
      output << std::endl;
    }


  output << "#include <pybind11/pybind11.h>" << std::endl;
  if (scope->has_std_namespace()) {
      output << "#include <pybind11/stl.h>" << std::endl;
  }
  if (scope->has_eigen_namespace()) {
      output << "#include <pybind11/eigen.h>" << std::endl;
      output << "#include <pybind11/eigen_tensor.h>" << std::endl;
  }
  for (auto &i : settings.includes) {
    output << "#include " << i << std::endl;
  }
  output << std::endl;
  output << "namespace py = pybind11;" << std::endl;
  output << std::endl;
}

} // namespace detail

/** Writer class
 *
 * The writer class manages the writing of interface files to
 * output streams.
 */
class Writer {

public:
  Writer(std::ostream& output)
      : output_(output) {}


    void write(const cxx::Scope* scope,
               const cxx::ASTNode* ast,
               const Settings settings) {

        detail::write_file_header(output_,
                                  scope,
                                  settings);
        if (ast) {
            ast->write_bindings(output_);
        }
    }


private:
  std::ostream& output_;
};

} // namespace pxx

#endif