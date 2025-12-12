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

# set build flags for HighFive 
set(HIGHFIVE_USE_BOOST OFF) 
set(HIGHFIVE_EXAMPLES OFF)
set(HIGHFIVE_BUILD_DOCS OFF)

FetchContent_Declare(external_highfive
   GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
   GIT_TAG v2.9.0
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_HIGHFIVE ${PROJECT_SOURCE_DIR}/external/ext2)
endif()
FetchContent_MakeAvailable(external_highfive)

FetchContent_Declare(external_isotope
   GIT_REPOSITORY https://github.com/Gregstrq/Isotope-data.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
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

FetchContent_Declare(external_periodic
   GIT_REPOSITORY https://github.com/Bowserinator/Periodic-Table-JSON.git
   GIT_TAG master
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
   EXCLUDE_FROM_ALL
   SOURCE_SUBDIR dummy
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_PERIODIC ${PROJECT_SOURCE_DIR}/external/ext5)
endif()
FetchContent_MakeAvailable(external_periodic)

FetchContent_Declare(external_libdedx
   GIT_REPOSITORY https://github.com/ir2-lab/libdedx.git
   GIT_TAG main
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
)
if(PACKAGE_BUILD)
   set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_LIBDEDX ${PROJECT_SOURCE_DIR}/external/ext6)
endif()
FetchContent_MakeAvailable(external_libdedx)

FetchContent_Declare(external_spline
   GIT_REPOSITORY https://github.com/ttk592/spline.git
   GIT_TAG master
   GIT_SUBMODULES_RECURSE FALSE
   GIT_SHALLOW TRUE
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

if (BUILD_GUI)
   FetchContent_Declare(external_qtdatabrowser
      GIT_REPOSITORY https://github.com/ir2-lab/QtDataBrowser.git
      GIT_TAG main
      GIT_SUBMODULES_RECURSE FALSE
      GIT_SHALLOW TRUE
   )
   if(PACKAGE_BUILD)
      set(FETCHCONTENT_SOURCE_DIR_QMATPLOTWIDGET ${PROJECT_SOURCE_DIR}/external/ext11)
      set(FETCHCONTENT_SOURCE_DIR_EXTERNAL_QTDATABROWSER ${PROJECT_SOURCE_DIR}/external/ext10)
   endif()
   FetchContent_MakeAvailable(external_qtdatabrowser)
endif()
