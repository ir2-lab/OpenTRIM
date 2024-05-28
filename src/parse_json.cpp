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
void to_json(ojson& j, const options& p);
void from_json(const ojson& j, options& p);

int options::parseJSON(std::istream& js)
{
    try {
        ojson j = ojson::parse(js,nullptr,true,true);
        *this = j.template get<options>();
        validate();
    }
    catch (const ojson::exception& e) {
        cerr << "Error reading json input:" << endl;
        cerr << e.what() << std::endl;
        return -1;
    }
    catch (const std::invalid_argument& e) {
        cerr << "Invalid option:" << endl;
        cerr << e.what() << endl;
        return -1;
    }
    return 0;
}

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(target::region,
                                   material_id,
                                   min, max)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(material::material_desc_t,
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
                                {mccore::SRIMlike, "SRIMlike"},
                                {mccore::MendenhallWeller, "MendenhallWeller"},
                                {mccore::MyFFP, "MyFFP"},
                                {mccore::AtomicSpacing, 1},
                                {mccore::Constant, 2},
                                {mccore::SRIMlike, 3},
                                {mccore::MendenhallWeller, 4},
                                {mccore::MyFFP, 5}
                            })   

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::scattering_calculation_t, {
                                {mccore::InvalidScatteringOption, nullptr},
                                {mccore::Corteo4bitTable, "Corteo4bitTable"},
                                {mccore::Corteo6bitTable, "Corteo6bitTable"},
                                {mccore::ZBL_MAGICK, "ZBL_MAGICK"},
                                {mccore::GCQuad, "GCQuad"},
                                {mccore::Corteo4bitTable, 0},
                                {mccore::Corteo6bitTable, 1},
                                {mccore::ZBL_MAGICK, 2},
                                {mccore::GCQuad, 3}
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

NLOHMANN_JSON_SERIALIZE_ENUM(mccore::straggling_model_t, {
                                {mccore::InvalidStraggling, nullptr},
                                {mccore::NoStraggling, "NoStraggling"},
                                {mccore::BohrStraggling, "BohrStraggling"},
                                {mccore::ChuStraggling, "ChuStraggling"},
                                {mccore::YangStraggling, "YangStraggling"},
                                {mccore::NoStraggling, 0},
                                {mccore::BohrStraggling, 1},
                                {mccore::ChuStraggling, 2},
                                {mccore::YangStraggling, 3}
                            })                                                                                        

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mccore::parameters,
                                   simulation_type, screening_type,
                                   scattering_calculation,
                                   flight_path_type, straggling_model,
                                   nrt_calculation,
                                   flight_path_const, min_energy, min_recoil_energy)

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcdriver::parameters,
                                          max_no_ions,
                                          threads, seeds)


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


void to_json(ojson& j, const options& p)
{
    j["Simulation"] = p.Simulation;
    j["IonBeam"] = p.IonBeam;
    j["Target"] = p.Target;
    j["Output"] = p.Output;
    j["Driver"] = p.Driver;
}
void from_json(const ojson& j, options& p)
{
    p = options();

    if (j.contains("Simulation"))
        p.Simulation = j["Simulation"];
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

void options::printJSON(std::ostream& os) const
{
    ojson j(*this);
    os << j.dump(4) << endl;
}

