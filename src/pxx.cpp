#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pxx/generator.h>

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(_pxx, m) {

  py::class_<pxx::Generator>(m, "Generator")
      .def(py::init<std::string, std::vector<std::string>>())
      .def("print_bindings", &pxx::Generator::print_bindings)
      .def("print_ast", &pxx::Generator::print_ast);
}
