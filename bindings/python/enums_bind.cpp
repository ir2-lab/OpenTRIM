/*
 * enums_bind.cpp
 *
 * Binds all OpenTRIM C++ enums and the Element class at module level.
 * Enum values are accessed as opentrim.Stopping.SRIM13, not as bare
 * opentrim.SRIM13 (no export_values() -- avoids collisions between enums).
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <functional>
#include <stdexcept>

#include "mccore.h"
#include "dedx.h"
#include "flight_path.h"
#include "ion_beam.h"
#include "tally.h"
#include "ion.h"

namespace py = pybind11;

void bind_enums(py::module_& m)
{
    py::enum_<mccore::simulation_type_t>(m, "SimulationType",
        "Type of ion transport simulation.")
        .value("FullCascade",  mccore::FullCascade,
               "Full damage cascade -- primary ions and all recoils followed.")
        .value("IonsOnly",     mccore::IonsOnly,
               "Primary ions only; recoil damage estimated by NRT.")
        .value("CascadesOnly", mccore::CascadesOnly,
               "Recoil cascades only; no primary ion transport.");

    /* mccore::None clashes with Python None -- exposed as NoScreening */
    py::enum_<mccore::screening_t>(m, "ScreeningType",
        "Nuclear scattering screening potential.")
        .value("NoScreening", mccore::None,
               "Unscreened Coulomb.")
        .value("Bohr",        mccore::Bohr)
        .value("KrC",         mccore::KrC,   "Krypton-Carbon.")
        .value("Moliere",     mccore::Moliere)
        .value("ZBL",         mccore::ZBL,   "Ziegler-Biersack-Littmark universal (default).")
        .value("ZBL_MAGIC",   mccore::ZBL_MAGIC, "ZBL with MAGIC analytic approximation.");

    py::enum_<dedx_calc::electronic_stopping_t>(m, "Stopping",
        "Electronic stopping model.")
        .value("Off",    dedx_calc::electronic_stopping_t::Off,
               "No electronic stopping.")
        .value("SRIM96", dedx_calc::electronic_stopping_t::SRIM96)
        .value("SRIM13", dedx_calc::electronic_stopping_t::SRIM13, "Default.")
        .value("DPASS",  dedx_calc::electronic_stopping_t::DPASS,
               "DPASS parametrization v21.06.");

    py::enum_<dedx_calc::electronic_straggling_t>(m, "Straggling",
        "Electronic energy straggling model.")
        .value("Off",  dedx_calc::electronic_straggling_t::Off,  "Default.")
        .value("Bohr", dedx_calc::electronic_straggling_t::Bohr)
        .value("Chu",  dedx_calc::electronic_straggling_t::Chu)
        .value("Yang", dedx_calc::electronic_straggling_t::Yang);

    py::enum_<mccore::nrt_calculation_t>(m, "NRT_Impl",
        "NRT vacancy calculation method in multi-element targets.")
        .value("NRT_element", mccore::NRT_element,
               "Ed of the struck atom (default, similar to SRIM).")
        .value("NRT_average", mccore::NRT_average,
               "Average Ed over all target atoms (Crocombette 2019).");

    py::enum_<flight_path_calc::flight_path_type_t>(m, "FlightPath",
        "Free-flight path selection algorithm.")
        .value("Constant", flight_path_calc::Constant,
               "Constant MFP (default). Value: Transport.flight_path_const [atomic radii].")
        .value("Variable", flight_path_calc::Variable,
               "Energy-dependent MFP sampled from pre-computed tables.");

    py::enum_<ion_beam::distribution_t>(m, "Distribution",
        "Statistical distribution for ion beam energy, position, and direction.")
        .value("SingleValue", ion_beam::SingleValue, "Delta distribution (default).")
        .value("Uniform",     ion_beam::Uniform)
        .value("Gaussian",    ion_beam::Gaussian,    "Normal distribution.");

    py::enum_<ion_beam::geometry_t>(m, "Geometry",
        "Ion source injection geometry.")
        .value("Surface", ion_beam::Surface, "Ions start at target surface (default).")
        .value("Volume",  ion_beam::Volume,  "Ions start inside target volume.");

    py::enum_<Event>(m, "Event",
        "Monte Carlo simulation events.  Used to configure UserTally triggers.")
        .value("NewSourceIon",    Event::NewSourceIon)
        .value("NewRecoil",       Event::NewRecoil)
        .value("Scattering",      Event::Scattering)
        .value("IonExit",         Event::IonExit)
        .value("IonStop",         Event::IonStop,          "Default UserTally trigger.")
        .value("BoundaryCrossing",Event::BoundaryCrossing)
        .value("Replacement",     Event::Replacement)
        .value("Vacancy",         Event::Vacancy)
        .value("CascadeComplete", Event::CascadeComplete)
        .value("NewFlightPath",   Event::NewFlightPath)
        .value("NEvent",          Event::NEvent,           "Sentinel -- total event count.")
        .value("Invalid",         Event::Invalid);

    py::class_<element_t>(m, "Element",
        R"(Atomic element used to specify ion beam species and target atom species.

Construction::

    opentrim.Element("He")  # from symbol, auto-fills Z=2, M=4.003
    opentrim.Element(2)     # from atomic number, auto-fills symbol and M

Attributes::

    symbol        str   element symbol
    atomic_number int   atomic number Z
    atomic_mass   float mean atomic mass [amu]
)")
        .def(py::init([](const std::string& sym) -> element_t {
            try {
                return element_t(sym);
            } catch (const std::invalid_argument& e) {
                throw py::value_error(e.what());
            }
        }), py::arg("symbol"),
        "Construct from element symbol.  Raises ValueError for unknown symbols.")

        .def(py::init([](int Z) -> element_t {
            try {
                return element_t(Z);
            } catch (const std::invalid_argument& e) {
                throw py::value_error(e.what());
            }
        }), py::arg("Z"),
        "Construct from atomic number Z (1-92).  Raises ValueError for out-of-range Z.")

        .def_readwrite("symbol",        &element_t::symbol)
        .def_readwrite("atomic_number", &element_t::atomic_number)
        .def_readwrite("atomic_mass",   &element_t::atomic_mass)

        .def("__repr__", [](const element_t& e) {
            return "Element(\"" + e.symbol + "\", Z=" + std::to_string(e.atomic_number)
                   + ", M=" + std::to_string(e.atomic_mass) + ")";
        })

        .def("__eq__", [](const element_t& a, const element_t& b) {
            return a.symbol == b.symbol
                   && a.atomic_number == b.atomic_number
                   && a.atomic_mass   == b.atomic_mass;
        })

        // keep consistent with __eq__ so equal elements hash equal - otherwise
        // defining __eq__ alone leaves Element unhashable (no sets / dict keys).
        .def("__hash__", [](const element_t& e) {
            size_t h = std::hash<std::string>{}(e.symbol);
            auto mix = [&h](size_t v) { h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2); };
            mix(std::hash<int>{}(e.atomic_number));
            mix(std::hash<float>{}(e.atomic_mass));
            return (py::ssize_t)h;
        });
}
