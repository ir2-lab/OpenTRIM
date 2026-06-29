/*
 * config_bind.cpp
 *
 * Binds opentrim.Config and all sub-structs / helper classes.
 *
 * PYBIND11_MAKE_OPAQUE must appear before pybind11/stl.h so that
 * compound vector fields return C++ references, not Python list copies.
 */

#include <pybind11/pybind11.h>

#include "target.h"
#include "user_tally.h"

PYBIND11_MAKE_OPAQUE(std::vector<atom::parameters>)
PYBIND11_MAKE_OPAQUE(std::vector<material::material_desc_t>)
PYBIND11_MAKE_OPAQUE(std::vector<target::region>)
PYBIND11_MAKE_OPAQUE(std::vector<user_tally::parameters>)

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "vector_casters.h"
#include "mcdriver.h"
#include <nlohmann/json.hpp>

namespace py = pybind11;

// formats a vector3 / ivector3 as "[x, y, z]" for __repr__ output
static std::string vec3_str(const vector3& v)
{
    return "[" + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", "
            + std::to_string(v.z()) + "]";
}
static std::string ivec3_str(const ivector3& v)
{
    return "[" + std::to_string(v.x()) + ", " + std::to_string(v.y()) + ", "
            + std::to_string(v.z()) + "]";
}

// looks up the Python name of a bound enum value (e.g. FullCascade) so __repr__
// prints the symbol rather than a raw integer.
template <typename E>
std::string enum_name(E value)
{
    return py::cast(value).attr("name").template cast<std::string>();
}

// adds __getitem__ / __setitem__ so any bound struct supports dict-style access
template <typename T>
void bind_dict_access(py::class_<T>& cls)
{
    cls.def("__getitem__", [](py::object self, const std::string& key) -> py::object {
        if (!py::hasattr(self, key.c_str()))
            throw py::key_error(key);
        return py::getattr(self, key.c_str());
    });
    cls.def("__setitem__", [](py::object self, const std::string& key, py::object val) {
        if (!py::hasattr(self, key.c_str()))
            throw py::key_error(key);
        py::setattr(self, key.c_str(), val);
    });
}

void bind_config(py::module_& m)
{
    // Each list is bound right after its element class (below), so pybind uses
    // the Python name (Atom, Material, ...) in the container signatures instead
    // of the raw C++ type - keeps the generated type stubs valid.

    py::class_<coord_sys> cs_cls(m, "CoordSys",
        "Coordinate system used to orient a UserTally.\n\n"
        "Defines a local frame by a z-axis direction and a vector in the xz-plane.");
    cs_cls
        .def(py::init<>())
        .def_property("origin",
            [](const coord_sys& c) { return c.origin; },
            [](coord_sys& c, const vector3& v) { c.origin = v; },
            "Origin of the coordinate system [nm].")
        .def_property("zaxis",
            [](const coord_sys& c) { return c.zaxis; },
            [](coord_sys& c, const vector3& v) { c.zaxis = v; },
            "Z-axis direction vector (unnormalized).")
        .def_property("xzvector",
            [](const coord_sys& c) { return c.xzvector; },
            [](coord_sys& c, const vector3& v) { c.xzvector = v; },
            "A vector in the xz-plane used to define the x-axis.")
        .def("__repr__", [](const coord_sys& c) {
            return "CoordSys(zaxis=" + vec3_str(c.zaxis)
                   + ", origin=" + vec3_str(c.origin) + ")";
        });
    bind_dict_access(cs_cls);

    py::class_<atom::parameters> atom_cls(m, "Atom",
        "Target atom species parameters.\n\n"
        "Used in Material.composition to define each element in the material::\n\n"
        "    atom = opentrim.Atom()\n"
        "    atom.element = opentrim.Element(\"Fe\")\n"
        "    atom.X  = 1.0    # atomic fraction\n"
        "    atom.Ed = 40.0   # displacement threshold [eV]\n"
        "    atom.El = 3.0    # lattice binding energy [eV]\n"
        "    atom.Es = 4.28   # surface binding energy [eV]\n"
        "    atom.Er = 40.0   # replacement threshold [eV]\n"
        "    atom.Rc = 0.946  # recombination radius [nm]");
    atom_cls
        .def(py::init<>())
        .def_readwrite("element", &atom::parameters::element)
        .def_readwrite("X",  &atom::parameters::X,  "Atomic fraction (sum over composition = 1).")
        .def_readwrite("Ed", &atom::parameters::Ed, "Displacement threshold energy [eV].")
        .def_readwrite("El", &atom::parameters::El, "Lattice binding energy [eV].")
        .def_readwrite("Es", &atom::parameters::Es, "Surface binding energy [eV].")
        .def_readwrite("Er", &atom::parameters::Er, "Replacement threshold energy [eV].")
        .def_readwrite("Rc", &atom::parameters::Rc, "Frenkel pair recombination radius [nm].")
        .def("__repr__", [](const atom::parameters& a) {
            return "Atom(element=" + a.element.symbol
                   + ", X=" + std::to_string(a.X)
                   + ", Ed=" + std::to_string(a.Ed) + ")";
        });
    bind_dict_access(atom_cls);
    py::bind_vector<std::vector<atom::parameters>>(m, "AtomList");

    py::class_<material::material_desc_t> mat_cls(m, "Material",
        "Target material descriptor::\n\n"
        "    mat = opentrim.Material()\n"
        "    mat.id      = \"Iron\"\n"
        "    mat.density = 7.874   # g/cm3\n"
        "    mat.composition.append(atom)\n"
        "    config.Target.materials.append(mat)");
    mat_cls
        .def(py::init<>())
        .def_readwrite("id",          &material::material_desc_t::id,
                       "User-defined material identifier.")
        .def_readwrite("density",     &material::material_desc_t::density,
                       "Mass density [g/cm3].")
        .def_readwrite("composition", &material::material_desc_t::composition,
                       "List of Atom objects (AtomList).")
        .def_readwrite("color",       &material::material_desc_t::color,
                       "Hex color string for GUI display, e.g. '#55aaff'.")
        .def("__repr__", [](const material::material_desc_t& m) {
            return "Material(id=\"" + m.id + "\", density=" + std::to_string(m.density)
                   + ", natoms=" + std::to_string(m.composition.size()) + ")";
        });
    bind_dict_access(mat_cls);
    py::bind_vector<std::vector<material::material_desc_t>>(m, "MaterialList");

    py::class_<target::region> region_cls(m, "Region",
        "Rectangular target region filled with a specific material::\n\n"
        "    region = opentrim.Region()\n"
        "    region.id          = \"FeLayer\"\n"
        "    region.material_id = \"Iron\"\n"
        "    region.origin      = [0.0, 0.0, 0.0]       # nm\n"
        "    region.size        = [100.0, 100.0, 100.0]  # nm\n"
        "    config.Target.regions.append(region)");
    region_cls
        .def(py::init<>())
        .def_readwrite("id",          &target::region::id,
                       "Region identifier.")
        .def_readwrite("material_id", &target::region::material_id,
                       "ID of the material filling this region.  Must match a Material.id.")
        .def_property("origin",
            [](const target::region& r) { return r.origin; },
            [](target::region& r, const vector3& v) { r.origin = v; },
            "Lower-left corner [x, y, z] in nm.")
        .def_property("size",
            [](const target::region& r) { return r.size; },
            [](target::region& r, const vector3& v) { r.size = v; },
            "Dimensions [Lx, Ly, Lz] in nm.")
        .def("__repr__", [](const target::region& r) {
            return "Region(id=\"" + r.id + "\", material_id=\"" + r.material_id + "\")";
        });
    bind_dict_access(region_cls);
    py::bind_vector<std::vector<target::region>>(m, "RegionList");

    py::class_<user_tally::bin_var_t> bins_cls(m, "UserTallyBins",
        "Bin edge vectors for a user-defined tally.\n\n"
        "Use assignment to set bin edges -- do not use .append() on individual fields\n"
        "because bin edge fields are plain std::vector<float> returned by value::\n\n"
        "    tally.bins.x = list(range(0, 101))   # correct -- assignment persists\n"
        "    tally.bins.E = [0, 1e3, 1e4, 1e5]   # correct\n\n"
        "    tally.bins.x.append(101)             # wrong -- mutates a copy, silently lost\n");
    bins_cls
        .def(py::init<>())
        .def_readwrite("x",        &user_tally::bin_var_t::x,        "Bin edges along x [nm].")
        .def_readwrite("y",        &user_tally::bin_var_t::y,        "Bin edges along y [nm].")
        .def_readwrite("z",        &user_tally::bin_var_t::z,        "Bin edges along z [nm].")
        .def_readwrite("r",        &user_tally::bin_var_t::r,        "Radial bin edges [nm].")
        .def_readwrite("rho",      &user_tally::bin_var_t::rho,      "Cylindrical rho bin edges [nm].")
        .def_readwrite("cosTheta", &user_tally::bin_var_t::cosTheta, "cos(theta) bin edges.")
        .def_readwrite("nx",       &user_tally::bin_var_t::nx,       "Direction cosine nx bin edges.")
        .def_readwrite("ny",       &user_tally::bin_var_t::ny,       "Direction cosine ny bin edges.")
        .def_readwrite("nz",       &user_tally::bin_var_t::nz,       "Direction cosine nz bin edges.")
        .def_readwrite("E",        &user_tally::bin_var_t::E,        "Energy bin edges [eV].")
        .def_readwrite("Tdam",     &user_tally::bin_var_t::Tdam,     "PKA damage energy bin edges [eV].")
        .def_readwrite("V",        &user_tally::bin_var_t::V,        "Vacancy count bin edges.")
        .def_readwrite("atom_id",  &user_tally::bin_var_t::atom_id,  "Atom species id bin edges.")
        .def_readwrite("recoil_id",&user_tally::bin_var_t::recoil_id,"Recoil generation id bin edges.")
        .def("__repr__", [](const user_tally::bin_var_t& b) {
            // list only the axes that actually have bin edges defined
            std::vector<std::string> ax;
            if (!b.x.empty())         ax.push_back("x");
            if (!b.y.empty())         ax.push_back("y");
            if (!b.z.empty())         ax.push_back("z");
            if (!b.r.empty())         ax.push_back("r");
            if (!b.rho.empty())       ax.push_back("rho");
            if (!b.cosTheta.empty())  ax.push_back("cosTheta");
            if (!b.nx.empty())        ax.push_back("nx");
            if (!b.ny.empty())        ax.push_back("ny");
            if (!b.nz.empty())        ax.push_back("nz");
            if (!b.E.empty())         ax.push_back("E");
            if (!b.Tdam.empty())      ax.push_back("Tdam");
            if (!b.V.empty())         ax.push_back("V");
            if (!b.atom_id.empty())   ax.push_back("atom_id");
            if (!b.recoil_id.empty()) ax.push_back("recoil_id");
            std::string s = "UserTallyBins(";
            if (ax.empty())
                s += "empty";
            else
                for (size_t i = 0; i < ax.size(); ++i)
                    s += (i ? ", " : "") + ax[i];
            return s + ")";
        });
    bind_dict_access(bins_cls);

    py::class_<user_tally::parameters> utally_cls(m, "UserTally",
        "User-defined tally configuration::\n\n"
        "    tally = opentrim.UserTally()\n"
        "    tally.id    = \"depth_profile\"\n"
        "    tally.event = opentrim.Event.Vacancy\n"
        "    tally.bins.x = list(range(0, 101))\n"
        "    config.UserTally.append(tally)");
    utally_cls
        .def(py::init<>())
        .def_readwrite("id",                &user_tally::parameters::id,
                       "Unique tally identifier.")
        .def_readwrite("description",       &user_tally::parameters::description,
                       "Optional short description.")
        .def_readwrite("event",             &user_tally::parameters::event,
                       "Event that triggers a score (opentrim.Event).")
        .def_readwrite("coordinate_system", &user_tally::parameters::coordinate_system,
                       "Local coordinate system for the tally bins (CoordSys).")
        .def_readwrite("bins",              &user_tally::parameters::bins,
                       "Bin edge definitions (UserTallyBins).")
        .def("__repr__", [](const user_tally::parameters& p) {
            return "UserTally(id=\"" + p.id + "\")";
        });
    bind_dict_access(utally_cls);
    py::bind_vector<std::vector<user_tally::parameters>>(m, "UserTallyList");

    py::class_<mccore::parameters> sim_cls(m, "SimulationParams",
        "Physics model selection.  Access via config.Simulation.");
    sim_cls
        .def(py::init<>())
        .def_readwrite("simulation_type",
                       &mccore::parameters::simulation_type,
                       "Simulation type (SimulationType).")
        .def_readwrite("screening_type",
                       &mccore::parameters::screening_type,
                       "Nuclear scattering potential (ScreeningType).")
        .def_readwrite("electronic_stopping",
                       &mccore::parameters::electronic_stopping,
                       "Electronic stopping model (Stopping).")
        .def_readwrite("electronic_straggling",
                       &mccore::parameters::electronic_straggling,
                       "Electronic straggling model (Straggling).")
        .def_readwrite("nrt_calculation",
                       &mccore::parameters::nrt_calculation,
                       "NRT vacancy method (NRT_Impl).")
        .def_readwrite("intra_cascade_recombination",
                       &mccore::parameters::intra_cascade_recombination,
                       "Allow intra-cascade Frenkel pair recombination.")
        .def("__repr__", [](const mccore::parameters& s) {
            return "SimulationParams(simulation_type=" + enum_name(s.simulation_type)
                   + ", electronic_stopping=" + enum_name(s.electronic_stopping) + ")";
        });
    bind_dict_access(sim_cls);

    py::class_<mccore::transport_options> tr_cls(m, "TransportParams",
        "Ion transport algorithm options.  Access via config.Transport.");
    tr_cls
        .def(py::init<>())
        .def_readwrite("flight_path_type",
                       &mccore::transport_options::flight_path_type,
                       "Flight path algorithm (FlightPath).")
        .def_readwrite("flight_path_const",
                       &mccore::transport_options::flight_path_const,
                       "Constant MFP value [atomic radii].  Used when FlightPath.Constant.")
        .def_readwrite("min_energy",
                       &mccore::transport_options::min_energy,
                       "Stop tracking ion below this energy [eV].")
        .def_readwrite("min_recoil_energy",
                       &mccore::transport_options::min_recoil_energy,
                       "Stop tracking recoil below this energy [eV].")
        .def_readwrite("min_scattering_angle",
                       &mccore::transport_options::min_scattering_angle,
                       "Skip collisions below this angle [degrees].")
        .def_readwrite("max_rel_eloss",
                       &mccore::transport_options::max_rel_eloss,
                       "Max relative energy loss per step (dE/E).")
        .def_property("mfp_range",
            [](const mccore::transport_options& t) -> std::vector<float> {
                return {t.mfp_range[0], t.mfp_range[1]};
            },
            [](mccore::transport_options& t, const std::vector<float>& v) {
                if (v.size() != 2)
                    throw std::invalid_argument("mfp_range requires exactly 2 values");
                t.mfp_range[0] = v[0];
                t.mfp_range[1] = v[1];
            },
            "MFP limits [min, max] in atomic radii.")
        .def("__repr__", [](const mccore::transport_options& t) {
            return "TransportParams(flight_path_type=" + enum_name(t.flight_path_type)
                   + ", min_energy=" + std::to_string(t.min_energy) + ")";
        });
    bind_dict_access(tr_cls);


    py::class_<ion_beam::energy_distribution_t> edist_cls(m, "EnergyDistribution",
        "Ion beam energy distribution.  Access via config.IonBeam.energy_distribution.");
    edist_cls
        .def(py::init<>())
        .def_readwrite("type",   &ion_beam::energy_distribution_t::type,
                       "Distribution type (Distribution).")
        .def_readwrite("center", &ion_beam::energy_distribution_t::center,
                       "Mean energy [eV].")
        .def_readwrite("fwhm",   &ion_beam::energy_distribution_t::fwhm,
                       "FWHM of energy distribution [eV].")
        .def("__repr__", [](const ion_beam::energy_distribution_t& e) {
            return "EnergyDistribution(type=" + enum_name(e.type)
                   + ", center=" + std::to_string(e.center) + " eV)";
        });
    bind_dict_access(edist_cls);

    
    py::class_<ion_beam::spatial_distribution_t> sdist_cls(m, "SpatialDistribution",
        "Ion beam spatial distribution.  Access via config.IonBeam.spatial_distribution.");
    sdist_cls
        .def(py::init<>())
        .def_readwrite("geometry", &ion_beam::spatial_distribution_t::geometry,
                       "Source geometry (Geometry).")
        .def_readwrite("type",     &ion_beam::spatial_distribution_t::type,
                       "Spatial distribution type (Distribution).")
        .def_readwrite("fwhm",     &ion_beam::spatial_distribution_t::fwhm,
                       "Spatial spread FWHM [nm].")
        .def_property("center",
            [](const ion_beam::spatial_distribution_t& s) { return s.center; },
            [](ion_beam::spatial_distribution_t& s, const vector3& v) { s.center = v; },
            "Mean starting position [x, y, z] in nm.")
        .def("__repr__", [](const ion_beam::spatial_distribution_t& s) {
            return "SpatialDistribution(geometry=" + enum_name(s.geometry)
                   + ", type=" + enum_name(s.type) + ")";
        });
    bind_dict_access(sdist_cls);

    py::class_<ion_beam::angular_distribution_t> adist_cls(m, "AngularDistribution",
        "Ion beam angular distribution.  Access via config.IonBeam.angular_distribution.");
    adist_cls
        .def(py::init<>())
        .def_readwrite("type", &ion_beam::angular_distribution_t::type,
                       "Angular distribution type (Distribution).")
        .def_property("center",
            [](const ion_beam::angular_distribution_t& a) { return a.center; },
            [](ion_beam::angular_distribution_t& a, const vector3& v) { a.center = v; },
            "Mean beam direction (unnormalized).  Default: [1, 0, 0] = along +x.")
        .def_readwrite("fwhm", &ion_beam::angular_distribution_t::fwhm,
                       "Angular spread FWHM [sr].")
        .def("__repr__", [](const ion_beam::angular_distribution_t& a) {
            return "AngularDistribution(type=" + enum_name(a.type)
                   + ", center=" + vec3_str(a.center) + ")";
        });
    bind_dict_access(adist_cls);

    py::class_<ion_beam::parameters> ib_cls(m, "IonBeamParams",
        "Ion beam source definition.  Access via config.IonBeam.");
    ib_cls
        .def(py::init<>())
        .def_readwrite("ion",                  &ion_beam::parameters::ion,
                       "Beam ion species (Element).")
        .def_readwrite("energy_distribution",  &ion_beam::parameters::energy_distribution,
                       "Energy distribution (EnergyDistribution).")
        .def_readwrite("spatial_distribution", &ion_beam::parameters::spatial_distribution,
                       "Spatial distribution (SpatialDistribution).")
        .def_readwrite("angular_distribution", &ion_beam::parameters::angular_distribution,
                       "Angular distribution (AngularDistribution).")
        .def("__repr__", [](const ion_beam::parameters& p) {
            return "IonBeamParams(ion=" + p.ion.symbol
                   + ", E0=" + std::to_string(p.energy_distribution.center) + " eV)";
        });
    bind_dict_access(ib_cls);

    py::class_<target::target_desc_t> tgt_cls(m, "TargetParams",
        "Target geometry and composition.  Access via config.Target::\n\n"
        "    config.Target.size       = [100.0, 100.0, 100.0]  # nm\n"
        "    config.Target.cell_count = [200, 1, 1]\n"
        "    config.Target.materials.append(mat)\n"
        "    config.Target.regions.append(region)");
    tgt_cls
        .def(py::init<>())
        .def_property("origin",
            [](const target::target_desc_t& t) { return t.origin; },
            [](target::target_desc_t& t, const vector3& v) { t.origin = v; },
            "Simulation box origin [x, y, z] in nm.")
        .def_property("size",
            [](const target::target_desc_t& t) { return t.size; },
            [](target::target_desc_t& t, const vector3& v) { t.size = v; },
            "Simulation box dimensions [Lx, Ly, Lz] in nm.")
        .def_property("cell_count",
            [](const target::target_desc_t& t) { return t.cell_count; },
            [](target::target_desc_t& t, const ivector3& v) { t.cell_count = v; },
            "Grid cell count [Nx, Ny, Nz].")
        .def_property("periodic_bc",
            [](const target::target_desc_t& t) { return t.periodic_bc; },
            [](target::target_desc_t& t, const ivector3& v) { t.periodic_bc = v; },
            "Periodic boundary flags [x, y, z].  1=periodic, 0=absorbing.")
        .def_readwrite("materials", &target::target_desc_t::materials,
                       "List of Material objects (MaterialList).")
        .def_readwrite("regions",   &target::target_desc_t::regions,
                       "List of Region objects (RegionList).")
        .def("__repr__", [](const target::target_desc_t& t) {
            return "TargetParams(size=" + vec3_str(t.size)
                   + ", cell_count=" + ivec3_str(t.cell_count)
                   + ", materials=" + std::to_string(t.materials.size())
                   + ", regions=" + std::to_string(t.regions.size()) + ")";
        });
    bind_dict_access(tgt_cls);

    py::class_<mcconfig::run_options> run_cls(m, "RunOptions",
        "Simulation run parameters.  Access via config.Run.");
    run_cls
        .def(py::init<>())
        .def_property("max_no_ions",
            [](const mcconfig::run_options& r) { return (long long)r.max_no_ions; },
            [](mcconfig::run_options& r, long long v) {
                if (v < 0) throw std::invalid_argument("max_no_ions must be >= 0");
                r.max_no_ions = (size_t)v;
            },
            "Number of ion histories to simulate.")
        .def_property("max_cpu_time",
            [](const mcconfig::run_options& r) { return (long long)r.max_cpu_time; },
            [](mcconfig::run_options& r, long long v) {
                if (v < 0) throw std::invalid_argument("max_cpu_time must be >= 0");
                r.max_cpu_time = (size_t)v;
            },
            "Max wall-clock time [s].  0 = unlimited.")
        .def_readwrite("threads", &mcconfig::run_options::threads,
                       "Worker threads.  <= 0 = auto (half of hardware_concurrency if >3 cores, else 1).")
        .def_property("seed",
            [](const mcconfig::run_options& r) { return (unsigned int)r.seed; },
            [](mcconfig::run_options& r, unsigned int v) { r.seed = v; },
            "RNG seed for reproducibility.")
        .def("__repr__", [](const mcconfig::run_options& r) {
            return "RunOptions(max_no_ions=" + std::to_string(r.max_no_ions)
                   + ", threads=" + std::to_string(r.threads) + ")";
        });
    bind_dict_access(run_cls);

    py::class_<mcconfig::output_options> out_cls(m, "OutputOptions",
        "Output file options.  Access via config.Output.");
    out_cls
        .def(py::init<>())
        .def_readwrite("title",               &mcconfig::output_options::title,
                       "Simulation title string.")
        .def_readwrite("outfilename",         &mcconfig::output_options::outfilename,
                       "Base name for the HDF5 output file (no .h5 extension).")
        .def_readwrite("storage_interval",    &mcconfig::output_options::storage_interval,
                       "HDF5 write interval [ms].")
        .def_readwrite("store_exit_events",   &mcconfig::output_options::store_exit_events,
                       "Store per-ion exit events.")
        .def_readwrite("store_pka_events",    &mcconfig::output_options::store_pka_events,
                       "Store PKA events.")
        .def_readwrite("store_damage_events", &mcconfig::output_options::store_damage_events,
                       "Store damage event positions.")
        .def_readwrite("store_dedx",          &mcconfig::output_options::store_dedx,
                       "Store electronic stopping tables in the output file.")
        .def("__repr__", [](const mcconfig::output_options& o) {
            return "OutputOptions(outfilename=\"" + o.outfilename + "\")";
        });
    bind_dict_access(out_cls);

    py::class_<mcconfig> cfg_cls(m, "Config",
        R"(Full simulation configuration.

Construction::

    config = opentrim.Config()            # all options at defaults
    config = opentrim.Config("sim.json")  # load from JSON file

Attribute access (recommended -- IDE autocomplete works)::

    config.Simulation.electronic_stopping = opentrim.Stopping.SRIM13
    config.Run.max_no_ions = 10000

Dict-style access (same fields, useful for programmatic access)::

    config["Run"]["threads"] = 4
    config["Simulation"]["electronic_stopping"] = opentrim.Stopping.SRIM13

Methods::

    config.validate()                # raises ValueError on invalid config
    config.to_json()                 # -> str, full config as JSON
    opentrim.Config.from_json(s)     # classmethod: construct from JSON string
    opentrim.Config.options_spec()   # -> str, JSON spec of all options
)");

    cfg_cls.def(py::init<>(),
        "Create a Config with all options at their default values.");

    cfg_cls.def(py::init([](const std::string& filename) {
        mcconfig cfg;
        std::ifstream f(filename);
        if (!f)
            throw py::value_error("Config: cannot open file \"" + filename + "\"");
        std::ostringstream err;
        if (cfg.parseJSON(f, /*doValidation=*/true, &err) != 0)
            throw py::value_error("Config: parse error in \"" + filename + "\": " + err.str());
        return cfg;
    }), py::arg("filename"),
    "Load a Config from a JSON file.  Raises ValueError on read or parse error.");

    cfg_cls
        .def_readwrite("Simulation", &mcconfig::Simulation)
        .def_readwrite("Transport",  &mcconfig::Transport)
        .def_readwrite("IonBeam",    &mcconfig::IonBeam)
        .def_readwrite("Target",     &mcconfig::Target)
        .def_readwrite("Run",        &mcconfig::Run)
        .def_readwrite("Output",     &mcconfig::Output)
        .def_readwrite("UserTally",  &mcconfig::UserTally,
                       "List of user-defined tallies (UserTallyList).");

    bind_dict_access(cfg_cls);

    cfg_cls
        .def("validate", [](mcconfig& cfg) {
            try {
                cfg.validate(/*AcceptIncomplete=*/false);
            } catch (const std::invalid_argument& e) {
                throw py::value_error(e.what());
            }
        },
        "Validate the configuration.  Raises ValueError with a description if invalid.")

        .def("to_json", [](const mcconfig& cfg) {
            return cfg.toJSON();
        },
        "Return the full configuration as a JSON string.")

        .def_static("from_json", [](const std::string& s) {
            // toJSON() outputs fields not in the JSON spec; strip them one by one
            nlohmann::json j;
            try {
                j = nlohmann::json::parse(s);
            } catch (const nlohmann::json::parse_error& e) {
                throw py::value_error(std::string("Config.from_json: invalid JSON: ") + e.what());
            }
            for (int attempts = 0; attempts < 32; ++attempts) {
                mcconfig cfg;
                std::istringstream ss(j.dump());
                std::ostringstream err;
                if (cfg.parseJSON(ss, /*doValidation=*/false, &err) == 0)
                    return cfg;
                // error path format: (/Section/field)
                std::string emsg = err.str();
                auto p1 = emsg.find('(');
                auto p2 = emsg.find(')');
                if (p1 == std::string::npos || p2 == std::string::npos)
                    throw py::value_error("Config.from_json: " + emsg);
                std::string path  = emsg.substr(p1 + 1, p2 - p1 - 1);
                auto slash        = path.rfind('/');
                if (slash == std::string::npos || slash == 0)
                    throw py::value_error("Config.from_json: " + emsg);
                std::string section = path.substr(1, slash - 1);
                std::string field   = path.substr(slash + 1);
                if (!j.contains(section) || !j[section].contains(field))
                    throw py::value_error("Config.from_json: " + emsg);
                j[section].erase(field);
            }
            throw py::value_error("Config.from_json: too many unrecognized fields in JSON");
        }, py::arg("json_string"),
        "Construct a Config from a JSON string.  Does not validate; call config.validate() before use.")

        .def_static("options_spec", []() {
            return mcconfig::options_spec();
        },
        "Return the JSON spec of all config options (types, ranges, descriptions).")

        .def("__repr__", [](const mcconfig& cfg) {
            return "Config(ion=" + cfg.IonBeam.ion.symbol
                   + ", max_no_ions=" + std::to_string(cfg.Run.max_no_ions)
                   + ", threads="    + std::to_string(cfg.Run.threads) + ")";
        });
}
