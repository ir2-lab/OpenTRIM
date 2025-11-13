#ifndef JSON_DEFS_P_H
#define JSON_DEFS_P_H

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
#include <nlohmann/ordered_map.hpp>

#include "mcdriver.h"

// Define a special json type for:
//   - keeping the order of elements
//   - use float for real numbers (to avoid e.g. 0.100001, etc)
using ojson = nlohmann::basic_json<nlohmann::ordered_map, std::vector, std::string, bool,
                                   std::int64_t, std::uint64_t, float>;

// define my macro for ojson
#define MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Type, ...)                                    \
    inline void to_json(ojson &nlohmann_json_j, const Type &nlohmann_json_t)                    \
    {                                                                                           \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__))                \
    }                                                                                           \
    inline void from_json(const ojson &nlohmann_json_j, Type &nlohmann_json_t)                  \
    {                                                                                           \
        const Type nlohmann_json_default_obj{};                                                 \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) \
    }

NLOHMANN_JSON_SERIALIZE_ENUM(mcconfig::option_type_t,
                             { { mcconfig::tInvalid, nullptr },
                               { mcconfig::tEnum, "enum" },
                               { mcconfig::tFloat, "float" },
                               { mcconfig::tInt, "int" },
                               { mcconfig::tBool, "bool" },
                               { mcconfig::tString, "string" },
                               { mcconfig::tVector, "vector" },
                               { mcconfig::tIntVector, "ivector" },
                               { mcconfig::tStruct, "struct" },
                               { mcconfig::tArray, "array" } })

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcdriver::run_data, start_time, end_time, ips, cpu_time,
                                          nthreads, run_ion_count, total_ion_count)

#endif // JSON_DEFS_P_H
