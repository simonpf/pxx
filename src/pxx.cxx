#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pxx/parser.h>
#include <pxx/python.h>

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(_pxx, m) {
    py::class_<pxx::Parser>(m, "Parser")
        .def(py::init<const std::string &, std::vector<std::string>>(),
             "filename"_a, py::arg("command_line_args") = std::vector<std::string>{})
        .def("parse", &pxx::Parser::parse);
    py::class_<pxx::ast::AST>(m, "TranslationUnit")
        .def("print", &pxx::ast::AST::print)
        .def("dump_ast", &pxx::ast::AST::dump_ast);
    py::class_<pxx::python::Module>(m, "Module")
        .def(py::init<std::string, std::vector<std::string>>())
        .def("render", &pxx::python::Module::render);
}



