#include "mcdriver.h"

#include <fstream>
#include <iostream>

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>

using std::cout;
using std::cerr;
using std::endl;

// Define a special json type for:
//   - keeping the order of elements
//   - use float for real numbers (to avoid e.g. 0.100001, etc)
using ojson = nlohmann::basic_json<nlohmann::ordered_map,
                         std::vector,
                         std::string,
                         bool,
                         std::int64_t,
                         std::uint64_t,
                         float>;


// json serialization of Eigen::AlignedVector3<T> types
namespace nlohmann {

template <typename T>
struct adl_serializer< Eigen::AlignedVector3<T> > {
    static void to_json(ojson& j, const Eigen::AlignedVector3<T>& V3) {
        j = {V3.x(), V3.y(), V3.z()};
    }

    static void from_json(const ojson& j, Eigen::AlignedVector3<T>& V3) {
        std::array<T,3> v = j.template get< std::array<T,3> >();
        V3 = Eigen::AlignedVector3<T>( v[0], v[1], v[2] );
    }
};

} // end namespace nlohmann

// define my macro for ojson
#define MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Type, ...)  \
    inline void to_json(ojson& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    inline void from_json(const ojson& nlohmann_json_j, Type& nlohmann_json_t) { const Type nlohmann_json_default_obj{}; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) }

// serialization of options struct
void to_json(ojson& j, const mcdriver::options& p);
void from_json(const ojson& j, mcdriver::options& p);

int mcdriver::options::parseJSON(std::istream& js, bool doValidation, std::ostream* os)
{
    if (os) {
        try {
            ojson j = ojson::parse(js,nullptr,true,true);
            *this = j.template get<mcdriver::options>();
            if (doValidation) validate();
        }
        catch (const ojson::exception& e) {
            *os << "Error reading json input:" << endl;
            *os << e.what() << std::endl;
            return -1;
        }
        catch (const std::invalid_argument& e) {
            *os << "Invalid option:" << endl;
            *os << e.what() << endl;
            return -1;
        }
    } else {
        ojson j = ojson::parse(js,nullptr,true,true);
        *this = j.template get<mcdriver::options>();
        if (doValidation) validate();
    }
    return 0;
}

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(target::region,
                                          id,
                                          material_id,
                                          min, max)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(material::material_desc_t,
                                   id,
                                   density, isMassDensity,
                                   Z, M, X, Ed, El, Es, Er)

NLOHMANN_JSON_SERIALIZE_ENUM(ion_beam::ion_distribution_t, {
                                {ion_beam::InvalidIonDistribution, nullptr},
                                {ion_beam::SurfaceRandom, "SurfaceRandom"},
                                {ion_beam::SurfaceCentered, "SurfaceCentered"},
                                {ion_beam::FixedPos, "FixedPos"},
                                {ion_beam::VolumeCentered, "VolumeCentered"},
                                {ion_beam::VolumeRandom, "VolumeRandom"},
                                {ion_beam::SurfaceRandom, 0},
                                {ion_beam::SurfaceCentered, 1},
                                {ion_beam::FixedPos, 2},
                                {ion_beam::VolumeCentered, 3},
                                {ion_beam::VolumeRandom, 4}
                            })

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ion_beam::parameters,
                                   ion_distribution,
                                   ionZ, ionM, ionE0,
                                   dir, pos)


NLOHMANN_JSON_SERIALIZE_ENUM(mccore::simulation_type_t, {
                                {mccore::InvalidSimulationType, nullptr},
                                {mccore::FullCascade, "FullCascade"},
                                {mccore::IonsOnly, "IonsOnly"},
                                {mccore::FullCascade, 0},
                                {mccore::FullCascade, 1},
                                {mccore::FullCascade, 2},
                                {mccore::IonsOnly, 3}
                            })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::nrt_calculation_t, {
                                {mccore::NRT_InvalidOption, nullptr},
                                {mccore::NRT_element, "NRT_element"},
                                {mccore::NRT_average, "NRT_average"},
                                {mccore::NRT_element, 0},
                                {mccore::NRT_average, 1}
                            })


NLOHMANN_JSON_SERIALIZE_ENUM(mccore::flight_path_type_t, {
                                {mccore::InvalidPath, nullptr},
                                {mccore::AtomicSpacing, "AtomicSpacing"},
                                {mccore::Constant, "Constant"},
                                {mccore::MendenhallWeller, "MendenhallWeller"},
                                {mccore::IPP, "IPP"},
                                {mccore::AtomicSpacing, 0},
                                {mccore::Constant, 1},
                                {mccore::MendenhallWeller, 2},
                                {mccore::IPP, 3}
                            })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::scattering_calculation_t,
                             {
                              {mccore::InvalidScatteringOption, nullptr},
                              {mccore::Corteo4bitTable, "Corteo4bitTable"},
                              {mccore::ZBL_MAGICK, "ZBL_MAGICK"},
                              {mccore::Corteo4bitTable, 0},
                              {mccore::ZBL_MAGICK, 1},
                              })

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::eloss_calculation_t,
                             {
                              {mccore::InvalidEnergyLoss, nullptr},
                              {mccore::EnergyLossOff, "Off"},
                              {mccore::EnergyLoss, "EnergyLoss"},
                              {mccore::EnergyLossAndStraggling, "EnergyLossAndStraggling"},
                              {mccore::EnergyLossOff, 0},
                              {mccore::EnergyLoss, 1},
                              {mccore::EnergyLossAndStraggling, 2},
                              })

NLOHMANN_JSON_SERIALIZE_ENUM(Screening, {
                                {Screening::None, "None"},
                                {Screening::LenzJensen, "LenzJensen"},
                                {Screening::KrC, "KrC"},
                                {Screening::Moliere, "Moliere"},
                                {Screening::ZBL, "ZBL"},
                                {Screening::None, 0},
                                {Screening::LenzJensen, 1},
                                {Screening::KrC, 2},
                                {Screening::Moliere, 3},
                                {Screening::ZBL, 4}
                            })

NLOHMANN_JSON_SERIALIZE_ENUM(StragglingModel, {
                                {StragglingModel::Invalid, nullptr},
                                {StragglingModel::Bohr, "BohrStraggling"},
                                {StragglingModel::Chu,  "ChuStraggling"},
                                {StragglingModel::Yang, "YangStraggling"},
                                {StragglingModel::Bohr, 0},
                                {StragglingModel::Chu,  1},
                                {StragglingModel::Yang, 2}
                            })

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mccore::parameters,
                                          simulation_type, screening_type,
                                          scattering_calculation,
                                          eloss_calculation, straggling_model,
                                          nrt_calculation)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mccore::transport_options,
                                          flight_path_type,
                                          flight_path_const, min_energy, min_recoil_energy,
                                          allow_sub_ml_scattering, max_mfp, max_rel_eloss)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcdriver::parameters,
                                          max_no_ions,
                                          threads, seed)


MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcdriver::output_options,
                                                title,
                                                OutputFileBaseName,
                                                storage_interval,
                                                store_transmitted_ions,
                                                store_range_3d,
                                                store_ion_paths,
                                                store_path_limit,
                                                store_recoil_cascades,
                                                store_path_limit_recoils,
                                                store_pka,
                                                store_dedx
                                                )

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(target::target_desc_t,
                                                materials,
                                                regions,
                                                cell_count,
                                                periodic_bc,
                                                cell_size)


void to_json(ojson& j, const mcdriver::options& p)
{
    j["Simulation"] = p.Simulation;
    j["Transport"] = p.Transport;
    j["IonBeam"] = p.IonBeam;
    j["Target"] = p.Target;
    j["Output"] = p.Output;
    j["Driver"] = p.Driver;
}
void from_json(const ojson& j, mcdriver::options& p)
{
    p = mcdriver::options();

    if (j.contains("Simulation"))
        p.Simulation = j["Simulation"];
    if (j.contains("Transport"))
        p.Transport = j["Transport"];
    if (j.contains("Output"))
        p.Output = j["Output"];
    if (j.contains(("IonBeam")))
        p.IonBeam = j["IonBeam"];
    if (j.contains(("Driver")))
        p.Driver = j["Driver"];
    if (!j.contains("Target")) {
        throw std::invalid_argument("Required section \"Target\" not found.");
    }
    p.Target = j["Target"];
}

void mcdriver::options::printJSON(std::ostream& os) const
{
    ojson j(*this);
    os << j.dump(4) << endl;
}

std::string mcdriver::options::toJSON() const
{
    std::ostringstream os;
    printJSON(os);
    return os.str();
}

