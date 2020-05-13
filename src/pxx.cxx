#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pxx/parser.h>
#include <pxx/python.h>

namespace py = pybind11;

PYBIND11_MODULE(_pxx, m) {
    py::class_<pxx::Parser>(m, "Parser")
        .def(py::init<const std::string &>())
        .def("print", &pxx::Parser::print)
        .def("parse", &pxx::Parser::parse);
    py::class_<pxx::ast::TranslationUnit>(m, "TranslationUnit")
        .def("print", &pxx::ast::TranslationUnit::print);
    py::class_<pxx::python::Module>(m, "Module")
        .def(py::init<std::string, std::vector<std::string>>())
        .def("render", &pxx::python::Module::render);
}



