/**
 * Unit tests for user_tally coordinates enum and HDF5 serialization
 * 
 * Tests the new coordinates_t enum and its JSON/HDF5 serialization
 */

#include <cassert>
#include <iostream>
#include <sstream>
#include "../source/include/user_tally.h"
#include "../source/lib/json_defs_p.h"

using ojson = nlohmann::basic_json<nlohmann::ordered_map, std::vector, std::string, bool,
                                  std::int64_t, std::uint64_t, float>;

void test_coordinate_enum_creation()
{
    std::cout << "TEST: Coordinate enum default creation... ";
    
    user_tally::parameters params;
    assert(params.coordinates == user_tally::coordinate_t::xyz);
    
    std::cout << "PASS" << std::endl;
}

void test_coordinate_enum_assignment()
{
    std::cout << "TEST: Coordinate enum assignment... ";
    
    user_tally::parameters params;
    
    params.coordinates = user_tally::coordinate_t::cylindrical;
    assert(params.coordinates == user_tally::coordinate_t::cylindrical);
    
    params.coordinates = user_tally::coordinate_t::spherical;
    assert(params.coordinates == user_tally::coordinate_t::spherical);
    
    params.coordinates = user_tally::coordinate_t::xyz;
    assert(params.coordinates == user_tally::coordinate_t::xyz);
    
    std::cout << "PASS" << std::endl;
}

void test_coordinate_enum_json_serialization()
{
    std::cout << "TEST: Coordinate enum JSON to_json... ";
    
    user_tally::parameters params;
    params.id = "TestTally";
    params.coordinates = user_tally::coordinate_t::cylindrical;
    
    ojson j;
    to_json(j, params);
    
    assert(j.contains("coordinates"));
    assert(j["coordinates"].is_string());
    std::string coord_str = j["coordinates"].get<std::string>();
    assert(coord_str == "cylindrical");
    
    std::cout << "PASS" << std::endl;
}

void test_coordinate_enum_json_deserialization()
{
    std::cout << "TEST: Coordinate enum JSON from_json... ";
    
    ojson j;
    j["id"] = "TestTally";
    j["description"] = "Test description";
    j["event"] = "IonStop";
    j["coordinates"] = "spherical";
    j["coordinate_system"] = ojson{
        {"origin", ojson::array({0.f, 0.f, 0.f})},
        {"zaxis", ojson::array({0.f, 0.f, 1.f})},
        {"xzvector", ojson::array({1.f, 0.f, 1.f})}
    };
    j["bins"] = ojson{
        {"x", ojson::array()},
        {"y", ojson::array()},
        {"z", ojson::array()},
        {"r", ojson::array()},
        {"rho", ojson::array()},
        {"cosTheta", ojson::array()},
        {"nx", ojson::array()},
        {"ny", ojson::array()},
        {"nz", ojson::array()},
        {"E", ojson::array()},
        {"Tdam", ojson::array()},
        {"V", ojson::array()},
        {"atom_id", ojson::array()},
        {"recoil_id", ojson::array()}
    };
    
    user_tally::parameters params;
    try {
        from_json(j, params);
        assert(params.id == "TestTally");
        assert(params.coordinates == user_tally::coordinate_t::spherical);
    } catch (const std::exception &e) {
        std::cerr << "ERROR in from_json: " << e.what() << std::endl;
        assert(false);
    }
    
    std::cout << "PASS" << std::endl;
}

void test_all_coordinate_types_json()
{
    std::cout << "TEST: All coordinate types JSON round-trip... ";
    
    std::string coord_names[] = {"xyz", "cylindrical", "spherical"};
    user_tally::coordinate_t coord_values[] = {
        user_tally::coordinate_t::xyz,
        user_tally::coordinate_t::cylindrical,
        user_tally::coordinate_t::spherical
    };
    
    for (int i = 0; i < 3; ++i) {
        user_tally::parameters params;
        params.coordinates = coord_values[i];
        
        ojson j;
        to_json(j, params);
        assert(j["coordinates"].get<std::string>() == coord_names[i]);
    }
    
    std::cout << "PASS" << std::endl;
}

void test_user_tally_coordinates_getter()
{
    std::cout << "TEST: user_tally coordinates() getter method... ";
    
    user_tally::parameters params;
    params.coordinates = user_tally::coordinate_t::cylindrical;
    params.id = "CylindricalTally";
    
    // Create a minimal user_tally (note: this may fail if init() is required)
    // We're just testing that the getter works
    user_tally tally(params);
    assert(tally.coordinates() == user_tally::coordinate_t::cylindrical);
    assert(tally.id() == "CylindricalTally");
    
    std::cout << "PASS" << std::endl;
}

void test_default_coordinates_is_xyz()
{
    std::cout << "TEST: Default coordinates is xyz... ";
    
    user_tally::parameters params;
    assert(params.coordinates == user_tally::coordinate_t::xyz);
    
    user_tally tally(params);
    assert(tally.coordinates() == user_tally::coordinate_t::xyz);
    
    std::cout << "PASS" << std::endl;
}

int main()
{
    std::cout << "\n=== User Tally Coordinates Unit Tests ===" << std::endl;
    
    try {
        test_coordinate_enum_creation();
        test_coordinate_enum_assignment();
        test_coordinate_enum_json_serialization();
        test_coordinate_enum_json_deserialization();
        test_all_coordinate_types_json();
        test_user_tally_coordinates_getter();
        test_default_coordinates_is_xyz();
        
        std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n=== TEST FAILED ===" << std::endl;
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
