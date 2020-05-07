#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>
#include <pxx/parser.h>

namespace py = pybind11;

PYBIND11_MODULE(pxx, m) {
    py::class_<pxx::Parser>(m, "Parser")
        .def(py::init<const std::string &>())
        .def("print", &pxx::Parser::print);
}



