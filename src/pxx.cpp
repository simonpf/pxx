#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <pxx/settings.h>
#include <pxx/cxx/translation_unit.h>

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(_pxx, m) {

py::class_<pxx::Settings>(m, "Settings")
    .def(py::init<>())
    .def_readwrite("header", &pxx::Settings::header)
    .def_readwrite("includes", &pxx::Settings::includes);
py::class_<pxx::cxx::TranslationUnit>(m, "TranslationUnit")
    .def(py::init<std::string, std::vector<std::string>>())
    .def("dump_ast", &pxx::cxx::TranslationUnit::dump_ast)
    .def("print_bindings", static_cast<std::string (pxx::cxx::TranslationUnit::*)(pxx::Settings)>(
             &pxx::cxx::TranslationUnit::print_bindings
             )
        )
    .def("print_bindings", static_cast<std::string (pxx::cxx::TranslationUnit::*)(std::string)>(
             &pxx::cxx::TranslationUnit::print_bindings
             )
    );
}
