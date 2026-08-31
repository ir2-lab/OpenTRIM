add_executable(test_scattering_calc 
    test_scattering_calc.cpp
    ${CMAKE_SOURCE_DIR}/source/include/scattering.h
)

target_include_directories(test_scattering_calc 
PRIVATE
    ${CMAKE_SOURCE_DIR}/source/include
)
target_link_libraries(test_scattering_calc
  PRIVATE 
    Eigen3::Eigen
    screened_coulomb
    ieee754_seq
    xs_zbl
    xs_bohr
    xs_krc
    xs_moliere
)

add_executable(test_event_handling
    test_event_handling.cpp
    ${CMAKE_SOURCE_DIR}/source/gui/cascadeassembler.cpp
)

target_include_directories(test_event_handling
PRIVATE
    ${CMAKE_SOURCE_DIR}/source/include
    ${CMAKE_SOURCE_DIR}/source/gui
)

target_compile_definitions(test_event_handling
PRIVATE
    CONFIG_JSON="${CMAKE_SOURCE_DIR}/test/opentrim/b1.json"
)

target_link_libraries(test_event_handling
    opentrim
)

add_test(NAME event_handling COMMAND test_event_handling)