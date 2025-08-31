include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/external)
set(FETCHCONTENT_QUIET FALSE)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)

option(PACKAGE_BUILD "Flag for building binary package without internet and localy downloaded externals." OFF)

FetchContent_Declare(external_cxxopts
   GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
   GIT_TAG v3.1.1
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(external_cxxopts_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext1)
else()
   FetchContent_MakeAvailable(external_cxxopts)
endif()

FetchContent_Declare(external_highfive
   GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
   GIT_TAG v2.9.0
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(external_highfive_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext2)
else()
   FetchContent_MakeAvailable(external_highfive)
endif()

FetchContent_Declare(external_isotope
   GIT_REPOSITORY https://github.com/Gregstrq/Isotope-data.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(external_isotope_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext3)
else()
   FetchContent_MakeAvailable(external_isotope)
endif()

FetchContent_Declare(external_json
   GIT_REPOSITORY https://github.com/nlohmann/json.git
   GIT_TAG v3.11.3
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(external_json_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext4)
else()
   FetchContent_MakeAvailable(external_json)
endif()

FetchContent_Declare(external_periodic
   GIT_REPOSITORY https://github.com/Bowserinator/Periodic-Table-JSON.git
   GIT_TAG master
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(external_periodic_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext5)
else()
   FetchContent_MakeAvailable(external_periodic)
endif()

FetchContent_Declare(external_libdedx
   GIT_REPOSITORY https://github.com/ir2-lab/libdedx.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
)
if(PACKAGE_BUILD)
   set(external_libdedx_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext6)
else()
   FetchContent_MakeAvailable(external_libdedx)
endif()

FetchContent_Declare(external_spline
   GIT_REPOSITORY https://github.com/ttk592/spline.git
   GIT_TAG master
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(external_spline_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext7)
else()
   FetchContent_MakeAvailable(external_spline)
endif()

FetchContent_Declare(external_screened_coulomb
   GIT_REPOSITORY https://github.com/ir2-lab/screened_coulomb.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
if(PACKAGE_BUILD)
   set(external_screened_coulomb_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext8)
else()
   FetchContent_MakeAvailable(external_screened_coulomb)
endif()

FetchContent_Declare(external_ieee754_seq
   GIT_REPOSITORY https://github.com/ir2-lab/ieee754_seq.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
if(PACKAGE_BUILD)
   set(external_ieee754_seq_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext9)
else()
   FetchContent_MakeAvailable(external_ieee754_seq)
endif()

if (BUILD_GUI)
   FetchContent_Declare(external_qmatplotwidget
      GIT_REPOSITORY https://gitlab.com/qdaq/qmatplotwidget.git
      GIT_TAG master
      GIT_SUBMODULES_RECURSE FALSE
      GIT_SHALLOW TRUE
      EXCLUDE_FROM_ALL
      SOURCE_SUBDIR dummy
   )
   if(PACKAGE_BUILD)
      set(external_qmatplotwidget_SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/ext10)
   else()
         FetchContent_MakeAvailable(external_qmatplotwidget)
   endif()
endif()
