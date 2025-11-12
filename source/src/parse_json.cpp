#include "mcdriver.h"
#include "json_defs_p.h"
#include "periodic_table.h"
#include "user_tally.h"
#include <iostream>
#include <sstream>

using std::cerr;
using std::cout;
using std::endl;

// json serialization of Eigen::AlignedVector3<T> types
namespace nlohmann {

template <typename T>
struct adl_serializer<Eigen::AlignedVector3<T>>
{
    static void to_json(ojson &j, const Eigen::AlignedVector3<T> &V3)
    {
        j = { V3.x(), V3.y(), V3.z() };
    }

    static void from_json(const ojson &j, Eigen::AlignedVector3<T> &V3)
    {
        std::array<T, 3> v = j.template get<std::array<T, 3>>();
        V3 = Eigen::AlignedVector3<T>(v[0], v[1], v[2]);
    }
};

} // end namespace nlohmann

// serialization of mcconfig struct
void to_json(ojson &j, const mcconfig &p);
void from_json(const ojson &j, mcconfig &p);

// validate json config agains option specs
int validate_json_config(const std::string &specs, const ojson &j);

int mcconfig::parseJSON(std::istream &js, bool doValidation, std::ostream *os)
{
    if (os) {
        try {
            ojson j = ojson::parse(js, nullptr, true, true);
            validate_json_config(options_spec(), j);
            *this = j.template get<mcconfig>();
            if (doValidation)
                validate();
        } catch (const ojson::exception &e) {
            *os << "Error reading json input:" << endl;
            *os << e.what() << std::endl;
            return -1;
        } catch (const std::invalid_argument &e) {
            *os << "Invalid option:" << endl;
            *os << e.what() << endl;
            return -1;
        }
    } else {
        ojson j = ojson::parse(js, nullptr, true, true);
        validate_json_config(options_spec(), j);
        *this = j.template get<mcconfig>();
        if (doValidation)
            validate();
    }
    return 0;
}

/* serialization of element_t struct
 *
 * This is needed so that an empty element is
 * always invalid input (without needing to call
 * validate() )
 *
 * This brakes the logic that json parsing checks
 * only for json grammar
 *
 * The options.set function needs to get a full element_t definition
 * in json. Otherwise it will throw error if you try to change
 * e.g. first the symbol and then the atomic_number
 *
 * TODO
 * Must be changed in the future to a more clear
 * logic
 * e.g. throw an invalid_input exception during parsing
 *
 */

void to_json(ojson &j, const element_t &p)
{
    j = ojson{ { "symbol", p.symbol },
               { "atomic_number", p.atomic_number },
               { "atomic_mass", p.atomic_mass } };
}

void check_element_def(const ojson &j, element_t &p)
{
    if (p.symbol.empty() && p.atomic_number <= 0) {
        std::stringstream msg;
        msg << "Undefined element. ";
        msg << "Specify either \"symbol\" or \"atomic_number\"";
        throw ojson::exception(ojson::other_error::create(1000, msg.str(), &j));
    }

    if (p.symbol.empty()) {
        if (p.atomic_number > dedx_interp::Zmax) {
            std::stringstream msg;
            msg << "Element with atomic_number Z=" << p.atomic_number;
            msg << " beyong the maximum possible Z=" << dedx_interp::Zmax;
            throw ojson::exception(ojson::other_error::create(1001, msg.str(), &j));
        }
        p.symbol = periodic_table::at(p.atomic_number).symbol;
    } else {
        auto &el = periodic_table::at(p.symbol);
        if (!el.is_valid()) {
            std::stringstream msg;
            msg << "Invalid element symbol=" << p.symbol;
            throw ojson::exception(ojson::other_error::create(1002, msg.str(), &j));
        }
        if (el.Z > dedx_interp::Zmax) {
            std::stringstream msg;
            msg << "Element " << p.symbol << "(Z=" << el.Z << ")";
            msg << "is beyong the maximum possible Z=" << dedx_interp::Zmax;
            throw ojson::exception(ojson::other_error::create(1002, msg.str(), &j));
        }
        if (p.atomic_number > 0 && el.Z != p.atomic_number) {
            std::stringstream msg;
            msg << "Incompatible element symbol " << p.symbol << "(Z=" << el.Z << ")";
            msg << " and specified atomic_number Z=" << p.atomic_number;
            throw ojson::exception(ojson::other_error::create(1002, msg.str(), &j));
        }
        p.atomic_number = el.Z;
    }

    if (p.atomic_mass == 0.0f)
        p.atomic_mass = periodic_table::at(p.atomic_number).mass;
}

void from_json(const ojson &nlohmann_json_j, element_t &nlohmann_json_t)
{
    const element_t nlohmann_json_default_obj{};
    NLOHMANN_JSON_FROM_WITH_DEFAULT(symbol);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(atomic_number);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(atomic_mass);
    check_element_def(nlohmann_json_j, nlohmann_json_t);
}

// enum serialization definitions

NLOHMANN_JSON_SERIALIZE_ENUM(ion_beam::distribution_t,
                             { { ion_beam::InvalidDistribution, nullptr },
                               { ion_beam::SingleValue, "SingleValue" },
                               { ion_beam::Uniform, "Uniform" },
                               { ion_beam::Gaussian, "Gaussian" } })

NLOHMANN_JSON_SERIALIZE_ENUM(ion_beam::geometry_t,
                             { { ion_beam::InvalidGeometry, nullptr },
                               { ion_beam::Surface, "Surface" },
                               { ion_beam::Volume, "Volume" } })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::simulation_type_t,
                             { { mccore::InvalidSimulationType, nullptr },
                               { mccore::FullCascade, "FullCascade" },
                               { mccore::IonsOnly, "IonsOnly" },
                               { mccore::CascadesOnly, "CascadesOnly" } })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::screening_t,
                             { { mccore::InvalidScreening, nullptr },
                               { mccore::None, "None" },
                               { mccore::Bohr, "Bohr" },
                               { mccore::KrC, "KrC" },
                               { mccore::Moliere, "Moliere" },
                               { mccore::ZBL, "ZBL" },
                               { mccore::ZBL_MAGIC, "ZBL_MAGIC" } })

NLOHMANN_JSON_SERIALIZE_ENUM(dedx_calc::electronic_stopping_t,
                             { { dedx_calc::electronic_stopping_t::Invalid, nullptr },
                               { dedx_calc::electronic_stopping_t::Off, "Off" },
                               { dedx_calc::electronic_stopping_t::SRIM96, "SRIM96" },
                               { dedx_calc::electronic_stopping_t::SRIM13, "SRIM13" },
                               { dedx_calc::electronic_stopping_t::DPASS, "DPASS" } })

NLOHMANN_JSON_SERIALIZE_ENUM(dedx_calc::electronic_straggling_t,
                             { { dedx_calc::electronic_straggling_t::Invalid, nullptr },
                               { dedx_calc::electronic_straggling_t::Off, "Off" },
                               { dedx_calc::electronic_straggling_t::Bohr, "Bohr" },
                               { dedx_calc::electronic_straggling_t::Chu, "Chu" },
                               { dedx_calc::electronic_straggling_t::Yang, "Yang" } })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::nrt_calculation_t,
                             { { mccore::NRT_InvalidOption, nullptr },
                               { mccore::NRT_element, "NRT_element" },
                               { mccore::NRT_average, "NRT_average" } })

NLOHMANN_JSON_SERIALIZE_ENUM(flight_path_calc::flight_path_type_t,
                             { { flight_path_calc::InvalidPath, nullptr },
                               { flight_path_calc::Constant, "Constant" },
                               { flight_path_calc::Variable, "Variable" } })

NLOHMANN_JSON_SERIALIZE_ENUM(Event,
                             { { Event::Invalid, nullptr },
                               { Event::NewSourceIon, "NewSourceIon" },
                               { Event::NewRecoil, "NewRecoil" },
                               { Event::Scattering, "Scattering" },
                               { Event::IonExit, "IonExit" },
                               { Event::IonStop, "IonStop" },
                               { Event::BoundaryCrossing, "BoundaryCrossing" },
                               { Event::Replacement, "Replacement" },
                               { Event::Vacancy, "Vacancy" },
                               { Event::CascadeComplete, "CascadeComplete" },
                               { Event::NewFlightPath, "NewFlightPath" },
                               { Event::NEvent, "NEvent" } })

// option struct serialization

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(target::region, id, material_id, origin, size)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(atom::parameters, element, X, Ed, El, Es, Er, Rc)

// MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
//     material::material_desc_t,
//     id,
//     density,
//     composition)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ion_beam::energy_distribution_t, type, center, fwhm)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ion_beam::spatial_distribution_t, geometry, type, center,
                                          fwhm)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ion_beam::angular_distribution_t, type, center, fwhm)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ion_beam::parameters, ion, energy_distribution,
                                          spatial_distribution, angular_distribution)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mccore::parameters, simulation_type, screening_type,
                                          electronic_stopping, electronic_straggling,
                                          nrt_calculation, intra_cascade_recombination,
                                          time_ordered_cascades, correlated_recombination,
                                          move_recoil, recoil_sub_ed)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mccore::transport_options, flight_path_type,
                                          flight_path_const, min_energy, min_recoil_energy,
                                          min_scattering_angle, max_rel_eloss, mfp_range)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcconfig::run_options, max_no_ions, max_cpu_time, threads,
                                          seed)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcconfig::output_options, title, outfilename,
                                          storage_interval, store_exit_events, store_pka_events,
                                          store_damage_events, store_dedx)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(coord_sys, origin, zaxis, xzvector)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(user_tally::parameters, id, description, event,
                                          coordinate_system, x, y, z, r, rho, cosTheta, nx, ny, nz,
                                          E, Tdam, V, atom_id, recoil_id)

// MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
//     target::target_desc_t,
//     origin,
//     size,
//     cell_count,
//     periodic_bc,
//     materials,
//     regions
//     )

void to_json(ojson &j, const material::material_desc_t &md)
{
    j["id"] = md.id;
    j["density"] = md.density;
    j["composition"] = md.composition;
    j["color"] = md.color;
}

void from_json(const ojson &nlohmann_json_j, material::material_desc_t &nlohmann_json_t)
{
    const material::material_desc_t nlohmann_json_default_obj{};
    NLOHMANN_JSON_FROM_WITH_DEFAULT(id);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(density);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(color);

    if (nlohmann_json_j["composition"].is_array())
        nlohmann_json_t.composition =
                nlohmann_json_j.value("composition", nlohmann_json_default_obj.composition);
    else if (nlohmann_json_j["composition"].is_object()) {
        atom::parameters atp;
        from_json(nlohmann_json_j["composition"], atp);
        nlohmann_json_t.composition.push_back(atp);
    }
}

void to_json(ojson &j, const target::target_desc_t &td)
{
    j["origin"] = td.origin;
    j["size"] = td.size;
    j["cell_count"] = td.cell_count;
    j["periodic_bc"] = td.periodic_bc;
    j["materials"] = td.materials;
    j["regions"] = td.regions;
}

void from_json(const ojson &nlohmann_json_j, target::target_desc_t &nlohmann_json_t)
{
    const target::target_desc_t nlohmann_json_default_obj{};
    NLOHMANN_JSON_FROM_WITH_DEFAULT(origin);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(size);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(cell_count);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(periodic_bc);
    if (nlohmann_json_j["materials"].is_array())
        nlohmann_json_t.materials =
                nlohmann_json_j.value("materials", nlohmann_json_default_obj.materials);
    else if (nlohmann_json_j["materials"].is_object()) {
        material::material_desc_t md;
        from_json(nlohmann_json_j["materials"], md);
        nlohmann_json_t.materials.push_back(md);
    }

    if (nlohmann_json_j["regions"].is_array())
        nlohmann_json_t.regions =
                nlohmann_json_j.value("regions", nlohmann_json_default_obj.regions);
    else if (nlohmann_json_j["regions"].is_object()) {
        target::region rd;
        from_json(nlohmann_json_j["regions"], rd);
        nlohmann_json_t.regions.push_back(rd);
    }
}

void to_json(ojson &j, const mcconfig &p)
{
    j["Simulation"] = p.Simulation;
    j["Transport"] = p.Transport;
    j["IonBeam"] = p.IonBeam;
    j["Target"] = p.Target;
    j["Output"] = p.Output;
    j["Run"] = p.Run;
    j["UserTally"] = p.UserTally;
}

void from_json(const ojson &j, mcconfig &p)
{
    p = mcconfig();

    if (j.contains("Simulation"))
        p.Simulation = j["Simulation"];
    if (j.contains("Transport"))
        p.Transport = j["Transport"];
    if (j.contains("Output"))
        p.Output = j["Output"];
    if (j.contains(("IonBeam")))
        p.IonBeam = j["IonBeam"];
    if (j.contains(("Run")))
        p.Run = j["Run"];
    if (!j.contains("Target")) {
        throw std::invalid_argument("Required section \"Target\" not found.");
    }
    p.Target = j["Target"];

    if (j.contains(("UserTally"))) {
        if (j["UserTally"].is_array())
            p.UserTally = j.value("UserTally", p.UserTally);
        else if (j["UserTally"].is_object()) {
            user_tally::parameters utp;
            from_json(j["UserTally"], utp);
            p.UserTally.push_back(utp);
        }
    }
}

// void from_json(const ojson &nlohmann_json_j, user_tally::parameters &nlohmann_json_t)
// {
//     const user_tally::parameters nlohmann_json_default_obj{};

//     NLOHMANN_JSON_FROM_WITH_DEFAULT(coordinates);
// }

void mcconfig::printJSON(std::ostream &os) const
{
    ojson j(*this);
    os << j.dump(4) << endl;
}

std::string mcconfig::toJSON() const
{
    std::ostringstream os;
    printJSON(os);
    return os.str();
}

bool mcconfig::set(const std::string &path, const std::string &json_str, std::ostream *os)
{
    if (os) {
        try {
            set_impl_(path, json_str);
        } catch (const ojson::exception &e) {
            *os << "Failed setting option " << path << " to " << json_str << endl
                << "Error: " << e.what() << endl
                << "exception id: " << e.id << endl;
            return false;
        } catch (const std::invalid_argument &e) {
            *os << "Failed setting option " << path << " to " << json_str << endl
                << "Error: " << e.what() << endl;
            return false;
        }
    } else
        set_impl_(path, json_str);
    return true;
}

bool mcconfig::get(const std::string &path, std::string &json_str, std::ostream *os) const
{
    if (os) {
        try {
            get_impl_(path, json_str);
        } catch (const ojson::exception &e) {
            *os << "Failed getting option " << path << endl
                << "error: " << e.what() << endl
                << "exception id: " << e.id << endl;
            return false;
        }
    } else
        get_impl_(path, json_str);
    return true;
}

void mcconfig::set_impl_(const std::string &path, const std::string &json_str)
{
    ojson j(*this);
    ojson::json_pointer ptr(path.c_str());
    ojson v = ojson::parse(json_str);
    j.at(ptr) = v;
    *this = j;
    validate(true);
}

void mcconfig::get_impl_(const std::string &path, std::string &json_str) const
{
    ojson j(*this);
    ojson::json_pointer ptr(path.c_str());
    ojson::const_reference vref = j.at(ptr);
    std::ostringstream ss;
    ss << vref.dump();
    json_str = ss.str();
}

ojson get_option_spec();

int validate_helper(const ojson &spec, const ojson &opt, const std::string &path = std::string())
{
    mcconfig::option_type_t type;
    spec["type"].get_to(type);
    std::string name;
    spec["name"].get_to(name);

    if (type == mcconfig::tStruct) {
        std::string next_path = path.empty() ? "/" : path + name + "/";
        for (auto it = spec["fields"].begin(); it != spec["fields"].end(); ++it) {
            const ojson &opt_spec = *it;
            // std::string next_path(path);
            // next_path += opt_spec["name"].template get<std::string>();
            // std::string typeName = opt_spec["type"].template get<std::string>();

            validate_helper(opt_spec, opt, next_path);
        }
        return 0;
    }

    // try if json key exists
    std::string jpath(path + name);
    ojson::json_pointer ptr(jpath);
    try {
        ojson::const_reference vref = opt.at(ptr);
    } catch (const ojson::out_of_range &e) {
        return 0;
    }

    // check the vale
    ojson::const_reference vref = opt.at(ptr);
    switch (type) {
    case mcconfig::tEnum: {
        std::vector<std::string> values;
        std::string v;
        vref.get_to(v);
        spec["values"].get_to(values);
        if (std::find(values.begin(), values.end(), v) == values.end()) {
            std::ostringstream msg;
            msg << "(" << jpath << ") ";
            msg << "Invalid enum value " << std::quoted(v) << std::endl;
            msg << "Valid options: ";
            auto i = values.begin();
            msg << std::quoted(*i++);
            for (; i != values.end(); ++i)
                msg << '|' << std::quoted(*i);
            msg << endl;
            throw std::invalid_argument(msg.str());
        }
    } break;
    case mcconfig::tFloat: {
        float v, vmin, vmax;
        vref.get_to(v);
        spec["min"].get_to(vmin);
        spec["max"].get_to(vmax);
        if (v > vmax || v < vmin) {
            std::ostringstream msg;
            msg << "(" << jpath << ") ";
            msg << "Out of bounds value " << v << endl;
            msg << "Valid range: [" << vmin << ", " << vmax << "]" << endl;
            throw std::invalid_argument(msg.str());
        }
    } break;
    case mcconfig::tVector: {
        std::vector<float> v;
        float vmin, vmax;
        vref.get_to(v);
        spec["min"].get_to(vmin);
        spec["max"].get_to(vmax);
        for (int i = 0; i < v.size(); ++i)
            if (v[i] > vmax || v[i] < vmin) {
                std::ostringstream msg;
                msg << "(" << jpath << "/" << i << ") ";
                msg << "Out of bounds value " << v[i] << endl;
                msg << "Valid range: [" << vmin << ", " << vmax << "]" << endl;
                throw std::invalid_argument(msg.str());
            }
    } break;
    case mcconfig::tIntVector: {
        std::vector<int> v;
        int vmin, vmax;
        vref.get_to(v);
        spec["min"].get_to(vmin);
        spec["max"].get_to(vmax);
        for (int i = 0; i < v.size(); ++i)
            if (v[i] > vmax || v[i] < vmin) {
                std::ostringstream msg;
                msg << "(" << jpath << "/" << i << ") ";
                msg << "Out of bounds value " << v[i] << endl;
                msg << "Valid range: [" << vmin << ", " << vmax << "]" << endl;
                throw std::invalid_argument(msg.str());
            }
    } break;
    case mcconfig::tInt: {
        int v, vmin, vmax;
        vref.get_to(v);
        spec["min"].get_to(vmin);
        spec["max"].get_to(vmax);
        if (v > vmax || v < vmin) {
            std::ostringstream msg;
            msg << "(" << jpath << ") ";
            msg << "Out of bounds value " << v << endl;
            msg << "Valid range: [" << vmin << ", " << vmax << "]" << endl;
            throw std::invalid_argument(msg.str());
        }
    } break;
    case mcconfig::tBool: {
        bool v;
        vref.get_to(v); // throws json exception if not bool
    } break;
    case mcconfig::tString: {
        std::string v;
        vref.get_to(v); // throws json exception if not string
    } break;
    default:
        assert(0);
        break;
    }
    return 0;
}

int validate_json_config(const std::string &specs, const ojson &j)
{
    std::istringstream is(specs);
    ojson json_specs = ojson::parse(is, nullptr, true, true);
    validate_helper(json_specs, j);
    return 0;
}
