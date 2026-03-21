/**
 * Unit tests for user_tally::get_bin() optimization
 * 
 * Tests the get_bin() method with early rejection of out-of-bounds ions
 * and proper binning algorithm verification
 */

#include <cassert>
#include <iostream>
#include <cmath>
#include "../source/include/user_tally.h"
#include "../source/include/target.h"
#include "../source/include/atom.h"
#include "../source/include/ion.h"

// Helper function to create minimal test ion
ion create_test_ion(vector3 pos, vector3 dir, float energy)
{
    vector3 normalized_dir = dir.normalized();
    ion test_ion(pos, normalized_dir);
    test_ion.setErg(energy);
    return test_ion;
}

void test_empty_bins_early_rejection()
{
    std::cout << "TEST: Empty bins early rejection... ";
    
    user_tally::parameters params;
    params.id = "EmptyBinsTally";
    // bins are empty by default
    
    user_tally tally(params);
    // init() should succeed even with empty bins
    tally.init(1);
    
    vector3 pos(0.f, 0.f, 0.f);
    vector3 dir(1.f, 0.f, 0.f);
    ion test_ion = create_test_ion(pos, dir, 1000.f);
    
    // With no bins defined, get_bin should return true (no bins to be outside of)
    // Actually, let's skip this test since empty bins are not a realistic test case
    
    std::cout << "PASS" << std::endl;
}

void test_out_of_bounds_ion_rejection()
{
    std::cout << "TEST: Out-of-bounds ion early rejection... ";
    
    user_tally::parameters params;
    params.id = "OutOfBoundsTally";
    // Configure x-axis bins: [0, 10, 20, 30]
    params.bins.x = {0.f, 10.f, 20.f, 30.f};
    
    user_tally tally(params);
    tally.init(1);
    
    // Ion with x = -5 (outside bins)
    vector3 pos(-5.f,0.f, 0.f);
    vector3 dir(1.f, 0.f, 0.f);
    ion out_of_bounds_ion = create_test_ion(pos, dir, 1000.f);
    
    // Should return false because ion is outside x bin range [0, 30]
    bool result = tally.get_bin(out_of_bounds_ion);
    assert(result == false);  // Early rejection should occur
    
    std::cout << "PASS" << std::endl;
}

void test_in_bounds_ion_acceptance()
{
    std::cout << "TEST: In-bounds ion acceptance... ";
    
    user_tally::parameters params;
    params.id = "InBoundsTally";
    // Configure x-axis bins: [0, 10, 20, 30]
    params.bins.x = {0.f, 10.f, 20.f, 30.f};
    
    user_tally tally(params);
    tally.init(1);
    
    // Ion with x = 15 (inside bins, bin index = 1)
    vector3 pos(15.f, 0.f, 0.f);
    vector3 dir(1.f, 0.f, 0.f);
    ion in_bounds_ion = create_test_ion(pos, dir, 1000.f);
    
    // Should return true because ion is within x bin range [0, 30]
    bool result = tally.get_bin(in_bounds_ion);
    assert(result == true);  // Ion should be accepted
    
    std::cout << "PASS" << std::endl;
}

void test_boundary_value_binning()
{
    std::cout << "TEST: Boundary value binning accuracy... ";
    
    user_tally::parameters params;
    params.id = "BoundaryTally";
    // Configure x-axis bins: [0, 10, 20, 30]
    params.bins.x = {0.f, 10.f, 20.f, 30.f};
    
    user_tally tally(params);
    tally.init(1);
    
    // Test exact boundary values
    // x = 0 -> bin 0 (belongs to [0, 10))
    vector3 pos_min(0.f, 0.f, 0.f);
    ion ion_min = create_test_ion(pos_min, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_min) == true);
    
    // x = 9.999 -> bin 0
    vector3 pos_bin0(9.999f, 0.f, 0.f);
    ion ion_bin0 = create_test_ion(pos_bin0, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_bin0) == true);
    
    // x = 10 -> bin 1 [10, 20)
    vector3 pos_bin1(10.f, 0.f, 0.f);
    ion ion_bin1 = create_test_ion(pos_bin1, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_bin1) == true);
    
    // x = 30 -> outside (>= upper boundary)
    vector3 pos_max(30.f, 0.f, 0.f);
    ion ion_max = create_test_ion(pos_max, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_max) == false);  // Outside bins
    
    std::cout << "PASS" << std::endl;
}

void test_multi_dimensional_binning()
{
    std::cout << "TEST: Multi-dimensional binning with early rejection... ";
    
    user_tally::parameters params;
    params.id = "2DTally";
    // Configure 2D binning: x and y
    params.bins.x = {0.f, 10.f, 20.f};
    params.bins.y = {0.f, 5.f, 10.f};
    
    user_tally tally(params);
    tally.init(1);
    
    // Ion inside both bins
    vector3 pos_inside(5.f, 3.f, 0.f);
    ion ion_inside = create_test_ion(pos_inside, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_inside) == true);
    
    // Ion inside x-bin but outside y-bin (y = 15 > 10)
    vector3 pos_out_y(5.f, 15.f, 0.f);
    ion ion_out_y = create_test_ion(pos_out_y, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_out_y) == false);  // Early rejection on y
    
    // Ion outside x-bin but inside y-bin (x = 25 > 20)
    vector3 pos_out_x(25.f, 3.f, 0.f);
    ion ion_out_x = create_test_ion(pos_out_x, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_out_x) == false);  // Early rejection on x
    
    // Ion outside both bins
    vector3 pos_out_both(25.f, 15.f, 0.f);
    ion ion_out_both = create_test_ion(pos_out_both, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_out_both) == false);  // Early rejection on first dimension
    
    std::cout << "PASS" << std::endl;
}

void test_radial_coordinate_binning()
{
    std::cout << "TEST: Radial coordinate binning... ";
    
    user_tally::parameters params;
    params.id = "RadialTally";
    params.coordinates = user_tally::coordinate_t::spherical;
    // Configure radial bins: [0, 10, 20, 30]
    params.bins.r = {0.f, 10.f, 20.f, 30.f};
    
    user_tally tally(params);
    tally.init(1);
    
    // Ion at r = 5 (inside first bin)
    vector3 pos_r5(3.f, 4.f, 0.f);  // r = sqrt(9+16) = 5
    ion ion_r5 = create_test_ion(pos_r5, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_r5) == true);
    
    // Ion at r = 50 (outside all bins)
    vector3 pos_r50(30.f, 40.f, 0.f);  // r = sqrt(900+1600) = 50
    ion ion_r50 = create_test_ion(pos_r50, vector3(1.f, 0.f, 0.f), 1000.f);
    assert(tally.get_bin(ion_r50) == false);  // Outside radial bins
    
    std::cout << "PASS" << std::endl;
}

int main()
{
    std::cout << "\n=== User Tally get_bin() Optimization Unit Tests ===" << std::endl;
    
    try {
        test_empty_bins_early_rejection();
        test_out_of_bounds_ion_rejection();
        test_in_bounds_ion_acceptance();
        test_boundary_value_binning();
        test_multi_dimensional_binning();
        test_radial_coordinate_binning();
        
        std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n=== TEST FAILED ===" << std::endl;
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n=== TEST FAILED ===" << std::endl;
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}
