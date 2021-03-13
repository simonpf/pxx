#define PY_SSIZE_T_CLEAN
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pxx/generator.h>

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(_pxx, m) {

    py::class_<pxx::Generator>(m, "Generator");
}
