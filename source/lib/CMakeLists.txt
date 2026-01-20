# Build the OpenTRIM library

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
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in
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
        version.cpp.in
)

# Generate options_spec.cpp from options_spec.json
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/options_spec.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_CURRENT_SOURCE_DIR}/options_spec.cpp.in
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/options_spec.cpp
    -DFILE_CONTENT=${CMAKE_CURRENT_SOURCE_DIR}/options_spec.json
    -DVAR_NAME="OPTIONS_SPEC"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/options_spec.json
        ${CMAKE_CURRENT_SOURCE_DIR}/options_spec.cpp.in
)

# Generate config_template.cpp from config_template.json
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config_template.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_CURRENT_SOURCE_DIR}/config_template.cpp.in
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/config_template.cpp
    -DFILE_CONTENT=${CMAKE_CURRENT_SOURCE_DIR}/config_template.json
    -DVAR_NAME="CONFIG_TEMPLATE"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/config_template.json
        ${CMAKE_CURRENT_SOURCE_DIR}/config_template.cpp.in
)

# Generate mcinfo_spec.cpp from mcinfo_spec.json
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mcinfo_spec.cpp
    COMMAND ${CMAKE_COMMAND}
    -DRUN_BUILD_CONFIG_FILE=1
    -DFILE_IN=${CMAKE_CURRENT_SOURCE_DIR}/mcinfo_spec.cpp.in
    -DFILE_OUT=${CMAKE_CURRENT_BINARY_DIR}/mcinfo_spec.cpp
    -DFILE_CONTENT=${CMAKE_CURRENT_SOURCE_DIR}/mcinfo_spec.json
    -DVAR_NAME="MCINFO_SPEC"
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildConfigFile.cmake
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/mcinfo_spec.json
        ${CMAKE_CURRENT_SOURCE_DIR}/mcinfo_spec.cpp.in
        
)

set(OPENTRIM_LIB_SOURCES 
    ion.cpp  
    cascade.cpp 
    target.cpp
    flight_path.cpp
    mccore.cpp   
    ion_beam.cpp
    event_stream.cpp
    h5serialize.cpp
    tally.cpp
    parse_json.cpp    
    mcdriver.cpp
    dedx.cpp
    straggling.cpp
    user_tally.cpp
    mcinfo.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/options_spec.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/config_template.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/mcinfo_spec.cpp
    periodic_table.cpp
    ../include/json_defs_p.h
)

set(OPENTRIM_LIB_HEADERS 
    ../include/scattering.h
    ../include/periodic_table.h
    ../include/ion.h 
    ../include/cascade.h
    ../include/random_vars.h
    ../include/target.h 
    ../include/flight_path.h
    ../include/dedx.h
    ../include/mccore.h 
    ../include/geometry.h
    ../include/ion_beam.h 
    ../include/arrays.h
    ../include/event_stream.h 
    ../include/tally.h 
    ../include/mcdriver.h 
    ../include/user_tally.h
    ../include/mcinfo.h
    ${external_screened_coulomb_SOURCE_DIR}/include/screened_coulomb.h
    ${external_ieee754_seq_SOURCE_DIR}/include/ieee754_seq.h
)

add_library(${LIB_TARGET} SHARED
    ${OPENTRIM_LIB_SOURCES}
    ${OPENTRIM_LIB_HEADERS}
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
    target_sources(${LIB_TARGET}
        PUBLIC FILE_SET HEADERS 
            BASE_DIRS 
              ../include 
              ${external_screened_coulomb_SOURCE_DIR}/include
              ${external_ieee754_seq_SOURCE_DIR}/include
            FILES
                ${OPENTRIM_LIB_HEADERS}
    )
else()
    set_property(TARGET ${LIB_TARGET}
        PROPERTY PUBLIC_HEADER
            ${OPENTRIM_LIB_HEADERS} 
    )
endif()

target_link_libraries(${LIB_TARGET} 
    PUBLIC 
        Eigen3::Eigen
        libdedx::dedx
    PRIVATE
        "-lm"
        nlohmann_json::nlohmann_json
        ${CMAKE_THREAD_LIBS_INIT} 
        ${CMAKE_PTHREAD_LIBS_INIT}
        hdf5::hdf5
        HighFive::Include        
        xs_zbl 
        xs_bohr 
        xs_krc 
        xs_moliere        
)

target_include_directories(${LIB_TARGET} 
PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${LIB_TARGET}>
PRIVATE
    ${external_spline_SOURCE_DIR}/src
)

include(CMakePackageConfigHelpers)

set(TARGETS_TO_EXPORT 
  ${LIB_TARGET} 
  xs_zbl 
  xs_bohr 
  xs_krc 
  xs_moliere
)

# Install the library, public headers and exported target
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
  install(TARGETS ${TARGETS_TO_EXPORT}
      EXPORT  ${PROJECT_NAME}Targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_TARGET}
  )
else()
  install(TARGETS ${TARGETS_TO_EXPORT}
      EXPORT  ${PROJECT_NAME}Targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_TARGET}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

# Install exported target file for find_package usage
set(OPENTRIM_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_TARGET})

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${OPENTRIM_CONFIG_INSTALL_DIR}
)

# Generate and install CMake package config files
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# A minimal config file that includes the exported targets. Create a template
# at cmake/OpentrimConfig.cmake.in if you want to add custom find logic.
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/OpenTRIMConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${OPENTRIM_CONFIG_INSTALL_DIR}
)

# Export targets to the build tree for other subprojects
export(EXPORT ${PROJECT_NAME}Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)