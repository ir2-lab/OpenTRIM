
# Generators build scattering table libs etc.
add_subdirectory(generators)

# Generate periodic_table.cpp
set(ISOTOPE_CSV ${external_isotope_SOURCE_DIR}/isotopes_data.csv)
set(PTABLE_CSV ${external_periodic_SOURCE_DIR}/PeriodicTableCSV.csv)
if(EXISTS ${ISOTOPE_CSV} AND EXISTS ${PTABLE_CSV})
    add_custom_command(OUTPUT periodic_table.cpp
        COMMAND genptable ${ISOTOPE_CSV} ${PTABLE_CSV} periodic_table.cpp
        DEPENDS genptable ${ISOTOPE_CSV} ${PTABLE_CSV}
    )
else()
    message(FATAL_ERROR "Periodic table and/or isotope data not Found!!!")
endif()

# Configure version.cpp
# all @###@ codes are replaced except for @GIT_TAG@
# which is configured during build
set(GIT_TAG "@GIT_TAG@")
configure_file(
    ${CMAKE_SOURCE_DIR}/source/src/version.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/version1.cpp
)

# Update the GIT_TAG in version.cpp
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_CURRENT_BINARY_DIR}/version1.cpp
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    -DFILE_CONTENT=${CMAKE_SOURCE_DIR}/git_tag
    -DVAR_NAME="GIT_TAG"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_SOURCE_DIR}/git_tag
        ${CMAKE_SOURCE_DIR}/source/src/version.cpp.in
)

# Generate options_spec.cpp from options_spec.json
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/options_spec.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_SOURCE_DIR}/source/src/options_spec.cpp.in
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/options_spec.cpp
    -DFILE_CONTENT=${CMAKE_SOURCE_DIR}/source/src/options_spec.json
    -DVAR_NAME="OPTIONS_SPEC"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_SOURCE_DIR}/source/src/options_spec.json
        ${CMAKE_SOURCE_DIR}/source/src/options_spec.cpp.in
)

# Generate config_template.cpp from config_template.json
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config_template.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_SOURCE_DIR}/source/src/config_template.cpp.in
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/config_template.cpp
    -DFILE_CONTENT=${CMAKE_SOURCE_DIR}/source/src/config_template.json
    -DVAR_NAME="CONFIG_TEMPLATE"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_SOURCE_DIR}/source/src/config_template.json
        ${CMAKE_SOURCE_DIR}/source/src/config_template.cpp.in
)

# Generate mcinfo_spec.cpp from mcinfo_spec.json
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mcinfo_spec.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_SOURCE_DIR}/source/src/mcinfo_spec.cpp.in
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/mcinfo_spec.cpp
    -DFILE_CONTENT=${CMAKE_SOURCE_DIR}/source/src/mcinfo_spec.json
    -DVAR_NAME="MCINFO_SPEC"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_SOURCE_DIR}/source/src/mcinfo_spec.json
        ${CMAKE_SOURCE_DIR}/source/src/mcinfo_spec.cpp.in
        
)

set (LIB_TARGET ${PROJECT_NAME_LOWERCASE})

add_library(${LIB_TARGET} SHARED
    src/ion.cpp  
    src/cascade.cpp 
    src/target.cpp
    src/flight_path.cpp
    src/mccore.cpp   
    src/ion_beam.cpp
    src/event_stream.cpp
    src/h5serialize.cpp
    src/tally.cpp
    src/parse_json.cpp    
    src/mcdriver.cpp
    src/dedx.cpp
    src/straggling.cpp
    src/user_tally.cpp
    src/mcinfo.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/options_spec.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/config_template.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/mcinfo_spec.cpp
    periodic_table.cpp

)

set(OPENTRIM_HEADERS 
    include/scattering.h
    include/periodic_table.h
    include/ion.h 
    include/cascade.h
    include/random_vars.h
    include/target.h 
    include/flight_path.h
    include/dedx.h
    include/mccore.h 
    include/geometry.h
    include/ion_beam.h 
    include/arrays.h
    include/event_stream.h 
    include/tally.h 
    include/mcdriver.h 
    include/user_tally.h
    include/mcinfo.h
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
    target_sources(${LIB_TARGET}
        PUBLIC FILE_SET HEADERS 
            BASE_DIRS include
            FILES
                ${OPENTRIM_HEADERS}
    )
else()
    set_property(TARGET ${LIB_TARGET}
        PROPERTY PUBLIC_HEADER
            ${OPENTRIM_HEADERS} 
    )
endif()
target_sources(${LIB_TARGET}
    PRIVATE 
        include/json_defs_p.h
)

target_link_libraries(${LIB_TARGET} 
    PUBLIC 
        Eigen3::Eigen
        nlohmann_json::nlohmann_json
        ieee754_seq
        screened_coulomb
        dedx
    PRIVATE
        "-lm"
        ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_PTHREAD_LIBS_INIT}
        ${HDF5_LIBRARIES}
        HighFive        
        xs_zbl xs_bohr xs_krc xs_moliere        
)

target_include_directories(${LIB_TARGET} 
PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include/${LIB_TARGET}>
PRIVATE
    ${external_spline_SOURCE_DIR}/src
)

# This is the CLI program
add_subdirectory(cli)

# Utility to generate options documentation 
add_executable(genoptionsdoc EXCLUDE_FROM_ALL
    generators/genoptionsdoc.cpp
)

target_link_libraries(genoptionsdoc
    ${LIB_TARGET}
)

add_custom_target(options_doc EXCLUDE_FROM_ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/options.dox.md"
)

add_custom_command(OUTPUT options.dox.md
    COMMAND genoptionsdoc 
    DEPENDS genoptionsdoc
)

# add_subdirectory(doc)

# install specs
set(installable_libs
    dedx
    xs_zbl xs_bohr xs_krc xs_moliere
    ${LIB_TARGET} )

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
    install(TARGETS ${installable_libs}
        LIBRARY
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME_LOWERCASE}
        RUNTIME
    )
else()
    install(TARGETS ${installable_libs}
        LIBRARY
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME_LOWERCASE}
        RUNTIME
    )
endif()

if (BUILD_GUI)
    add_subdirectory(gui)
endif () ## BUILD GUI