# Python bindings for OpenTRIM using pybind11.
# pybind11 is fetched via FetchContent in cmake/Externals.cmake.
# Links against the opentrim library target - no Qt dependency.

pybind11_add_module(_opentrim_core MODULE
    bindings.cpp
    enums_bind.cpp
    config_bind.cpp
    driver_bind.cpp
    info_bind.cpp
)

target_link_libraries(_opentrim_core PRIVATE
    opentrim
    nlohmann_json::nlohmann_json
)

target_include_directories(_opentrim_core PRIVATE
    ${PROJECT_SOURCE_DIR}/source/include
)

if(SKBUILD)
    # For Python installs, place OpenTRIM's private shared libraries next to
    # the extension module and use $ORIGIN so the dynamic loader finds them.
    # libdedx and HDF5 remain external runtime dependencies.
    set_target_properties(_opentrim_core PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
    set_target_properties(opentrim PROPERTIES
        BUILD_RPATH  "$ORIGIN;$<TARGET_FILE_DIR:libdedx::dedx>"
        INSTALL_RPATH "$ORIGIN;$<TARGET_FILE_DIR:libdedx::dedx>"
    )

    install(TARGETS _opentrim_core DESTINATION opentrim)
    install(FILES
        $<TARGET_FILE:opentrim>
        $<TARGET_FILE:xs_zbl>
        $<TARGET_FILE:xs_bohr>
        $<TARGET_FILE:xs_krc>
        $<TARGET_FILE:xs_moliere>
        DESTINATION opentrim
    )
    install(FILES opentrim/__init__.py opentrim/py.typed DESTINATION opentrim)
    # ship the generated type stubs (PEP 561) so pip-installed wheels get
    # IDE autocomplete - none present is fine
    install(DIRECTORY opentrim/ DESTINATION opentrim FILES_MATCHING PATTERN "*.pyi")
else()
    # Plain CMake build: libopentrim and xs_* are installed by
    # source/lib/CMakeLists.txt. _opentrim_core uses the global RPATH.
    install(TARGETS _opentrim_core DESTINATION opentrim)
    install(FILES opentrim/__init__.py opentrim/py.typed DESTINATION opentrim)
    # ship any generated type stubs (PEP 561) - none present is fine
    install(DIRECTORY opentrim/ DESTINATION opentrim FILES_MATCHING PATTERN "*.pyi")
endif()
