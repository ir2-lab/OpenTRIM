include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/external)
set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)

option(PACKAGE_BUILD "Flag for building binary package without internet and localy downloaded externals." OFF)

FetchContent_Declare(external_CLI11
   GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
   GIT_TAG v2.6.1
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_CLI11 ${PROJECT_SOURCE_DIR}/external/ext1)
endif()
FetchContent_MakeAvailable(external_CLI11)

# set build flags and then include HighFive 
set(HIGHFIVE_FIND_HDF5 Off)
set(HIGHFIVE_UNIT_TESTS OFF)
FetchContent_Declare(external_highfive
   GIT_REPOSITORY https://github.com/highfive-devs/highfive.git
   GIT_TAG v3.3.0
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_HIGHFIVE ${PROJECT_SOURCE_DIR}/external/ext2)
endif()
# FetchContent_MakeAvailable(external_highfive)
# We follow the method from this link
#   https://stackoverflow.com/questions/65527126/disable-install-for-fetchcontent
# so that highfive is NOT installed when we install opentrim
FetchContent_GetProperties(external_highfive)
if(NOT external_highfive_POPULATED)
  FetchContent_Populate(external_highfive)
  add_subdirectory(${external_highfive_SOURCE_DIR} ${external_highfive_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Isotope-data has no release tags; pin to a specific commit for reproducible
# builds (this is the master HEAD as of 2026-09-01).
# GIT_SHALLOW must be off: not all servers allow shallow-fetching a raw commit.
FetchContent_Declare(external_isotope
   GIT_REPOSITORY https://github.com/Gregstrq/Isotope-data.git
   GIT_TAG 9dd2180ba3bc6caf67063a59601af3e0960edbae
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW FALSE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_ISOTOPE ${PROJECT_SOURCE_DIR}/external/ext3)
endif()
FetchContent_MakeAvailable(external_isotope)

FetchContent_Declare(external_json
   GIT_REPOSITORY https://github.com/nlohmann/json.git
   GIT_TAG v3.11.3
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
#   EXCLUDE_FROM_ALL
#   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_JSON ${PROJECT_SOURCE_DIR}/external/ext4)
endif()
FetchContent_MakeAvailable(external_json)

# Periodic-Table-JSON tags (e.g. v.4.0.0) dropped PeriodicTableCSV.csv, which
# genptable needs; only master still ships it. Pin to a commit (master HEAD as
# of 2026-09-01) for reproducible builds.
FetchContent_Declare(external_periodic
   GIT_REPOSITORY https://github.com/Bowserinator/Periodic-Table-JSON.git
   GIT_TAG ea41119626581350fdcdd9c873de233645a43023
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW FALSE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_PERIODIC ${PROJECT_SOURCE_DIR}/external/ext5)
endif()
FetchContent_MakeAvailable(external_periodic)

# ttk592/spline has no release tags; pin to a commit (master HEAD as of
# 2026-09-01) for reproducible builds.
FetchContent_Declare(external_spline
   GIT_REPOSITORY https://github.com/ttk592/spline.git
   GIT_TAG 5894beaf91e9adbfdbe5c6c9a1c60770e380e8e8
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW FALSE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_SPLINE ${PROJECT_SOURCE_DIR}/external/ext7)
endif()
FetchContent_MakeAvailable(external_spline)

FetchContent_Declare(external_screened_coulomb
   GIT_REPOSITORY https://github.com/ir2-lab/screened_coulomb.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_SCREENED_COULOMB ${PROJECT_SOURCE_DIR}/external/ext8)
endif()
FetchContent_MakeAvailable(external_screened_coulomb)

FetchContent_Declare(external_ieee754_seq
   GIT_REPOSITORY https://github.com/ir2-lab/ieee754_seq.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_IEEE754_SEQ ${PROJECT_SOURCE_DIR}/external/ext9)
endif()
FetchContent_MakeAvailable(external_ieee754_seq)

if(OPENTRIM_BUILD_PYTHON)
   # Prefer a system pybind11 (>= 2.9.1); fall back to a pinned copy.
   set(PYBIND11_FINDPYTHON ON)
   find_package(pybind11 2.9.1 CONFIG QUIET)
   if(NOT pybind11_FOUND)
      FetchContent_Declare(external_pybind11
         GIT_REPOSITORY https://github.com/pybind/pybind11.git
         GIT_TAG        v2.13.6
         GIT_SUBMODULES_RECURSE FALSE
         GIT_SHALLOW    TRUE
      )
      if(PACKAGE_BUILD)
         set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_PYBIND11 ${PROJECT_SOURCE_DIR}/external/ext10)
      endif()
      FetchContent_MakeAvailable(external_pybind11)
   else()
      message(STATUS "Using system pybind11 ${pybind11_VERSION}")
   endif()
endif()
