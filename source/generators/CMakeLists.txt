# Generate scattering tables for different
# screening potentials and link them into
# shared libraries

add_executable(gen_scattering_tbl EXCLUDE_FROM_ALL
    gen_scattering_tbl.cpp
)

target_include_directories(gen_scattering_tbl 
    PRIVATE
    ../include
)

target_link_libraries(gen_scattering_tbl PRIVATE
    Eigen3::Eigen
    CLI11
    screened_coulomb
    ieee754_seq
)

function(add_xs_library screeningName)
    add_custom_command(OUTPUT xs_${screeningName}_data.cpp
        COMMAND gen_scattering_tbl -s ${screeningName} 
        DEPENDS gen_scattering_tbl
    )
    add_library(xs_${screeningName} SHARED
        ${CMAKE_CURRENT_BINARY_DIR}/xs_${screeningName}_data.cpp
    )
endfunction()

add_xs_library(zbl)
add_xs_library(bohr)
add_xs_library(krc)
add_xs_library(moliere)

# Generator utility for periodic_table.cpp
add_executable(genptable EXCLUDE_FROM_ALL
    genptable.cpp
)

