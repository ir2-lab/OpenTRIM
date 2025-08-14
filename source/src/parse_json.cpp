#include "mcdriver.h"
#include "json_defs_p.h"
#include "periodic_table.h"
#include "user_tally.h"
#include <iostream>

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

int mcconfig::parseJSON(std::istream &js, bool doValidation, std::ostream *os)
{
    if (os) {
        try {
            ojson j = ojson::parse(js, nullptr, true, true);
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

NLOHMANN_JSON_SERIALIZE_ENUM(dedx_calc::eloss_calculation_t,
                             { { dedx_calc::InvalidEnergyLoss, nullptr },
                               { dedx_calc::EnergyLossOff, "EnergyLossOff" },
                               { dedx_calc::EnergyLoss, "EnergyLoss" },
                               { dedx_calc::EnergyLossAndStraggling, "EnergyLossAndStraggling" } })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::nrt_calculation_t,
                             { { mccore::NRT_InvalidOption, nullptr },
                               { mccore::NRT_element, "NRT_element" },
                               { mccore::NRT_average, "NRT_average" } })

NLOHMANN_JSON_SERIALIZE_ENUM(flight_path_calc::flight_path_type_t,
                             { { flight_path_calc::InvalidPath, nullptr },
                               { flight_path_calc::Constant, "Constant" },
                               { flight_path_calc::MHW, "MHW" },
                               { flight_path_calc::FullMC, "FullMC" } })

NLOHMANN_JSON_SERIALIZE_ENUM(Screening,
                             { { Screening::Invalid, nullptr },
                               { Screening::None, "None" },
                               { Screening::LenzJensen, "LenzJensen" },
                               { Screening::KrC, "KrC" },
                               { Screening::Moliere, "Moliere" },
                               { Screening::ZBL, "ZBL" },
                               { Screening::ZBL_MAGIC, "ZBL_MAGIC" } })

NLOHMANN_JSON_SERIALIZE_ENUM(StragglingModel,
                             { { StragglingModel::Invalid, nullptr },
                               { StragglingModel::Bohr, "BohrStraggling" },
                               { StragglingModel::Chu, "ChuStraggling" },
                               { StragglingModel::Yang, "YangStraggling" } })

NLOHMANN_JSON_SERIALIZE_ENUM(StoppingModel,
                             { { StoppingModel::Invalid, nullptr },
                               { StoppingModel::SRIM96, "SRIM96" },
                               { StoppingModel::SRIM13, "SRIM13" },
                               { StoppingModel::DPASS22, "DPASS22" } })

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

NLOHMANN_JSON_SERIALIZE_ENUM(user_tally::coordinate_t,
                             { { user_tally::Invalid, nullptr },
                               { user_tally::xyz, "xyz" },
                               { user_tally::cyl, "cyl" },
                               { user_tally::sph, "sph" },
                               })

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
                                          eloss_calculation, stopping_model, straggling_model,
                                          nrt_calculation, intra_cascade_recombination,
                                          correlated_recombination, move_recoil, recoil_sub_ed)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mccore::transport_options, flight_path_type,
                                          flight_path_const, min_energy, min_recoil_energy,
                                          min_scattering_angle, max_rel_eloss, mfp_range)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcconfig::run_options, max_no_ions, max_cpu_time, threads,
                                          seed)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcconfig::output_options, title, outfilename,
                                          storage_interval, store_exit_events, store_pka_events,
                                          store_dedx)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(user_tally::parameters, id, event, coordinates, x, y, z, rho, phi, r, theta, zaxis, xzvec, org, vx, vy, vz, vrho, vphi, vr, vtheta)

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
}

void from_json(const ojson &nlohmann_json_j, material::material_desc_t &nlohmann_json_t)
{
    const material::material_desc_t nlohmann_json_default_obj{};
    NLOHMANN_JSON_FROM_WITH_DEFAULT(id);
    NLOHMANN_JSON_FROM_WITH_DEFAULT(density);

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

    if (j.contains(("UserTally")))
        p.UserTally = j["UserTally"];
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
