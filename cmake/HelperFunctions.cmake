
add_executable(genptable EXCLUDE_FROM_ALL
    source/generators/genptable.cpp
)

set(ISOTOPE_CSV ${external_isotope_SOURCE_DIR}/isotopes_data.csv)
set(PTABLE_CSV ${external_periodic_SOURCE_DIR}/PeriodicTableCSV.csv)
set(PTABLE_DATA_CPP periodic_table.cpp)

if(EXISTS ${ISOTOPE_CSV} AND EXISTS ${PTABLE_CSV})
    add_custom_command(OUTPUT ${PTABLE_DATA_CPP}
        COMMAND genptable ${ISOTOPE_CSV} ${PTABLE_CSV} ${PTABLE_DATA_CPP}
        DEPENDS genptable
    )
else()
    message(FATAL_ERROR "Periodic table and/or isotope data not Found!!!")
endif()

add_executable(gen_scattering_tbl EXCLUDE_FROM_ALL
    source/generators/gen_scattering_tbl.cpp
)

target_include_directories(gen_scattering_tbl PRIVATE
    ${external_cxxopts_SOURCE_DIR}/include
    ${external_screened_coulomb_SOURCE_DIR}/include)

function(add_xs_library screeningName)
    add_custom_command(OUTPUT xs_${screeningName}_data.cpp
        COMMAND gen_scattering_tbl -s ${screeningName} DEPENDS gen_scattering_tbl
    )
    add_library(xs_${screeningName} SHARED
        ${CMAKE_CURRENT_BINARY_DIR}/xs_${screeningName}_data.cpp
    )
endfunction()
