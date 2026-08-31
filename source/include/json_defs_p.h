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
        const Type nlohmann_json_default_obj{ };                                                \
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

MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(mcdriver::run_data, start_time, end_time, ions_per_cpu_s,
                                          cpu_time_s, nthreads, run_ion_count, total_ion_count)

// return mcconfig options specs as a ojson object
const ojson &json_options_spec();

/**
 * @brief Returns the specification sub-tree for a single option, selected by its JSON pointer
 *
 * @a config must be a fully materialized OpenTRIM configuration (e.g. as produced by
 * `ojson(mcconfig)`), i.e. all top-level sections present. @a jptr is resolved against both
 * the global options spec (returned by json_options_spec()) and @a config itself: every
 * struct field and array index along the path must exist in @a config, and the path must not
 * try to descend below a scalar or vector-typed option.
 *
 * @param config a valid, fully materialized OpenTRIM configuration
 * @param jptr the json pointer of the option to get the specs for
 * @param os optional stream to receive error messages
 * @return the matching spec node, copied from json_options_spec(), or a null ojson on error
 *         (if @a os is given)
 *
 * @throws std::invalid_argument if @a jptr is invalid and @a os is nullptr
 */
ojson spec_for_path(const ojson &config, const ojson::json_pointer &jptr,
                    std::ostream *os = nullptr);

inline const char *toString(mcconfig::option_type_t t)
{
    switch (t) {
    case mcconfig::tEnum:
        return "Enumerator";
    case mcconfig::tFloat:
        return "Real Number";
    case mcconfig::tInt:
        return "Integer";
    case mcconfig::tBool:
        return "Boolean";
    case mcconfig::tString:
        return "String";
    case mcconfig::tVector:
        return "Real Vector";
    case mcconfig::tIntVector:
        return "Integer Vector";
    case mcconfig::tStruct:
        return "Option Group";
    case mcconfig::tArray:
        return "Options Array";
    default:
        return "invalid";
    }
}

#endif // JSON_DEFS_P_H
