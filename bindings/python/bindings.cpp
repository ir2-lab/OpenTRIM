#include <pybind11/pybind11.h>

#include "mcdriver.h"

namespace py = pybind11;

// Forward declarations - implemented in separate translation units (added per PR)
// void bind_enums(py::module_&);   // A-2
// void bind_config(py::module_&);  // A-2
// void bind_driver(py::module_&);  // A-3
// void bind_info(py::module_&);    // A-4

PYBIND11_MODULE(_opentrim_core, m)
{
    m.doc() = "Python bindings for the OpenTRIM Monte Carlo ion transport simulator";
    m.attr("__version__") = mcdriver::version_info().version;

    // Registration calls added per PR:
    // bind_enums(m);   // A-2
    // bind_config(m);  // A-2
    // bind_driver(m);  // A-3
    // bind_info(m);    // A-4
}
