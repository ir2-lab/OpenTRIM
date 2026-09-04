# Python bindings for OpenTRIM using pybind11.
#
# Two build modes:
#  * in-tree    - added via add_subdirectory() from the top-level project;
#                 the `opentrim` and `pybind11` targets already exist.
#  * standalone - configured directly (cmake -S bindings/python ...); consumes
#                 an installed OpenTRIM via find_package() and a system pybind11.
#                 Used to build the distro `python3-opentrim` package.
#
# Links against the opentrim library target - no Qt dependency.

option(OPENTRIM_PYTHON_STANDALONE
    "Build the Python bindings as a standalone project against an installed OpenTRIM"
    OFF)

# Absolute install directory for the `opentrim` Python package.  When empty the
# destination is the interpreter's site-packages (standalone mode) or the
# prefix-relative `opentrim/` (in-tree / wheel builds).  Ignored for wheel
# (SKBUILD) builds, where scikit-build-core owns the layout.
set(OPENTRIM_PYTHON_INSTALL_DIR "" CACHE PATH
    "Install directory for the opentrim Python package")

if(NOT TARGET opentrim OR OPENTRIM_PYTHON_STANDALONE)
    set(_opentrim_py_standalone TRUE)
endif()

if(_opentrim_py_standalone)
    # cmake_minimum_required()/project() are only called here so that including
    # this file via add_subdirectory() does not lower the top-level project's
    # minimum (which is 3.23).  3.18 is needed for Python's Development.Module.
    cmake_minimum_required(VERSION 3.18)
    project(opentrim-python LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # No version arg: version lockstep with libopentrim is enforced by the
    # package (e.g. the OBS spec's `Requires: libopentrim = %{version}`).
    find_package(OpenTRIM CONFIG REQUIRED)

    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 2.9.1 CONFIG REQUIRED)
    find_package(Python 3 REQUIRED COMPONENTS Interpreter Development.Module)

    set(_opentrim_lib OpenTRIM::opentrim)
    if(OPENTRIM_PYTHON_INSTALL_DIR)
        set(_py_dest "${OPENTRIM_PYTHON_INSTALL_DIR}/opentrim")
    else()
        set(_py_dest "${Python_SITEARCH}/opentrim")
    endif()

    message(STATUS "OpenTRIM Python bindings: standalone build")
    message(STATUS "  OpenTRIM   ${OpenTRIM_VERSION}")
    message(STATUS "  pybind11   ${pybind11_VERSION}")
    message(STATUS "  Python     ${Python_VERSION} (${Python_EXECUTABLE})")
    message(STATUS "  install to ${_py_dest}")
else()
    set(_opentrim_lib opentrim)
    if(OPENTRIM_PYTHON_INSTALL_DIR AND NOT SKBUILD)
        set(_py_dest "${OPENTRIM_PYTHON_INSTALL_DIR}/opentrim")
    else()
        set(_py_dest opentrim)
    endif()
endif()

pybind11_add_module(_opentrim_core MODULE
    bindings.cpp
    enums_bind.cpp
    config_bind.cpp
    driver_bind.cpp
    info_bind.cpp
)

target_link_libraries(_opentrim_core PRIVATE ${_opentrim_lib})

install(TARGETS _opentrim_core DESTINATION "${_py_dest}")
install(FILES opentrim/__init__.py opentrim/py.typed DESTINATION "${_py_dest}")
# ship the checked-in type stubs (PEP 561) for editor auto-completion.
# regenerate after changing the bound API with:
#   pybind11-stubgen opentrim -o bindings/python
install(DIRECTORY opentrim/ DESTINATION "${_py_dest}" FILES_MATCHING PATTERN "*.pyi")

if(_opentrim_py_standalone)
    # libopentrim + libxs_* come from the system 'libopentrim' package and are
    # on the default loader path - no RPATH needed on the extension module.
elseif(SKBUILD)
    # For Python (wheel) installs, place OpenTRIM's private shared libraries
    # next to the extension module and use $ORIGIN so the dynamic loader finds
    # them.  libdedx and HDF5 remain external runtime dependencies.
    set_target_properties(_opentrim_core PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
    set_target_properties(opentrim PROPERTIES
        BUILD_RPATH  "$ORIGIN;$<TARGET_FILE_DIR:libdedx::dedx>"
        INSTALL_RPATH "$ORIGIN;$<TARGET_FILE_DIR:libdedx::dedx>"
    )
    install(FILES
        $<TARGET_FILE:opentrim>
        $<TARGET_FILE:xs_zbl>
        $<TARGET_FILE:xs_bohr>
        $<TARGET_FILE:xs_krc>
        $<TARGET_FILE:xs_moliere>
        DESTINATION "${_py_dest}"
    )
else()
    # Plain in-tree CMake build: libopentrim and xs_* are installed by
    # source/lib/CMakeLists.txt.  _opentrim_core uses the global RPATH.
endif()
