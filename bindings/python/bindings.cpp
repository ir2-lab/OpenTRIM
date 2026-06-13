#include <pybind11/pybind11.h>

#include "mcdriver.h"

namespace py = pybind11;

// forward declarations -- implemented in separate translation units
void bind_enums(py::module_&);
void bind_config(py::module_&);
void bind_driver(py::module_&);
void bind_info(py::module_&);

PYBIND11_MODULE(_opentrim_core, m)
{
    m.doc() = "Python bindings for the OpenTRIM Monte Carlo ion transport simulator";
    m.attr("__version__") = mcdriver::version_info().version;

    bind_enums(m);
    bind_config(m); // keep before bind_driver - Driver.init()/config() use Config
    bind_driver(m);
    bind_info(m); // keep after bind_driver - Info(driver) consumes a Driver
}
