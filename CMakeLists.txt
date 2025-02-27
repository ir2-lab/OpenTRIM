cmake_minimum_required(VERSION 3.12.0)
    
project(OpenTRIM 
        VERSION 1.0.0
        DESCRIPTION "Monte-Carlo simulation of ion transport in materials"
        HOMEPAGE_URL https://gitlab.com/ir2-lab/
        LANGUAGES C CXX)

set(PROJECT_NAME_LOWERCASE opentrim)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option (BUILD_GUI "build gui" ON)

message (STATUS "${PROJECT_NAME} version ${PROJECT_VERSION}")

string(TIMESTAMP CMAKE_BUILD_TIME UTC)

message (STATUS "Build time ${CMAKE_BUILD_TIME}")

add_compile_options("-march=native")

add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}"
                        PROJECT_VERSION="${PROJECT_VERSION}"
                        PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
                        SYSTEM_ID="${CMAKE_CXX_PLATFORM_ID}"
                        COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
                        COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}"
                        BUILD_TIME="${CMAKE_BUILD_TIME}")

include (GNUInstallDirs) 

# by default, install into $HOME/.local (not /usr/local), so that no root access (and sudo!!) is needed
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if((CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (NOT CMAKE_CROSSCOMPILING))
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}" CACHE PATH "Default install path" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Default install path" FORCE)
  endif()
endif()
message (STATUS "Install prefix =  ${CMAKE_INSTALL_PREFIX}")

include(Externals)
include(HelperFunctions)

if(PACKAGE_BUILD)
   set(CMAKE_POSITION_INDEPENDENT_CODE ON)
elseif((CMAKE_SYSTEM_NAME STREQUAL "Linux") AND (NOT CMAKE_CROSSCOMPILING))
    # use install rpath, no need to use LD_LIBRARY_PATH 
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
endif()
message (STATUS "Install rpath =  ${CMAKE_INSTALL_RPATH}")

find_package(Threads REQUIRED)

find_package(Eigen3 3.4 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})
message(STATUS "Eigen3 include ${EIGEN3_INCLUDE_DIR}")

find_package(HDF5 REQUIRED COMPONENTS C)
include_directories(${HDF5_INCLUDE_DIRS})
message(STATUS "HDF5 include: ${HDF5_INCLUDE_DIRS}")
message(STATUS "HDF5 libs: ${HDF5_LIBRARIES}")

include_directories(source/include)

file(GLOB gendedx_files CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/source/dedx/*.cpp)

add_library(dedx SHARED
    source/src/dedx.cpp
    source/src/straggling.cpp
    ${gendedx_files}
)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
    target_sources(dedx
        PUBLIC FILE_SET HEADERS
            BASE_DIRS source/include
            FILES 
                source/include/corteo.h
                source/include/dedx.h
    )
else()
    set_property(TARGET dedx
        PROPERTY PUBLIC_HEADER
            source/include/corteo.h
            source/include/dedx.h
    )
endif()

add_dedx_library(zbl)
add_dedx_library(lj)
add_dedx_library(krc)
add_dedx_library(moliere)

set (LIB_TARGET ${PROJECT_NAME_LOWERCASE})
set (EXE_TARGET ${PROJECT_NAME_LOWERCASE}_exe)

add_library(${LIB_TARGET} SHARED
    ${PTABLE_DATA_CPP}
    source/src/ion.cpp   
    source/src/target.cpp
    source/src/flight_path_calc.cpp
    source/src/mccore.cpp   
    source/src/ion_beam.cpp
    source/src/event_stream.cpp
    source/src/h5serialize.cpp
    source/src/tally.cpp
    source/src/parse_json.cpp    
    source/src/mcdriver.cpp
    source/src/dedx_calc.cpp
)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
    target_sources(${LIB_TARGET}
        PUBLIC FILE_SET HEADERS 
            BASE_DIRS source/include
            FILES
                source/include/corteo_xs.h
                source/include/xs.h
                source/include/periodic_table.h
                source/include/ion.h 
                source/include/cascade_queue.h
                source/include/random_vars.h
                source/include/target.h 
                source/include/flight_path_calc.h
                source/include/dedx_calc.h
                source/include/mccore.h 
                source/include/geometry.h
                source/include/ion_beam.h 
                source/include/arrays.h
                source/include/event_stream.h 
                source/include/tally.h 
                source/include/mcdriver.h 
    )
else()
    set_property(TARGET ${LIB_TARGET}
        PROPERTY PUBLIC_HEADER
            source/include/corteo_xs.h
            source/include/xs.h
            source/include/periodic_table.h
            source/include/ion.h 
            source/include/cascade_queue.h
            source/include/random_vars.h
            source/include/target.h 
            source/include/flight_path_calc.h
            source/include/dedx_calc.h
            source/include/mccore.h 
            source/include/geometry.h
            source/include/ion_beam.h 
            source/include/arrays.h
            source/include/event_stream.h 
            source/include/tally.h 
            source/include/mcdriver.h 
    )
endif()
target_sources(${LIB_TARGET}
    PRIVATE 
        source/include/json_defs_p.h
)

set(PROJECT_LIBS
    "-lm"
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_PTHREAD_LIBS_INIT}
    ${HDF5_LIBRARIES}
    dedx
    xs_zbl xs_lj xs_krc xs_moliere
)

target_link_libraries(${LIB_TARGET} ${PROJECT_LIBS})

target_include_directories(${LIB_TARGET} PRIVATE
    ${external_json_SOURCE_DIR}/single_include/ 
    ${external_highfive_SOURCE_DIR}/include)

add_executable(${EXE_TARGET}
    source/main.cpp
)
# adjust exe name to be the same as the library's
set_property(TARGET ${EXE_TARGET} PROPERTY OUTPUT_NAME ${PROJECT_NAME_LOWERCASE})

target_include_directories(${EXE_TARGET} PRIVATE
    ${external_cxxopts_SOURCE_DIR}/include)

target_link_libraries(${EXE_TARGET}
    ${LIB_TARGET}
)

# add_subdirectory(doc)

set(installable_libs
    dedx
    xs_zbl xs_lj xs_krc xs_moliere
    ${LIB_TARGET} )

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.23)
    install(TARGETS ${installable_libs} ${EXE_TARGET}
        LIBRARY
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME_LOWERCASE}
        RUNTIME
    )
else()
    install(TARGETS ${installable_libs} ${EXE_TARGET}
        LIBRARY
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME_LOWERCASE}
        RUNTIME
    )
endif()
if (BUILD_GUI)

add_subdirectory(source/gui)

endif () ## BUILD GUI





