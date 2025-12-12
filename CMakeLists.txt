cmake_minimum_required(VERSION 3.31.3)
    
project(OpenTRIM 
        VERSION 1.1.0
        DESCRIPTION "Monte-Carlo simulation of ion transport in materials"
        HOMEPAGE_URL https://gitlab.com/ir2-lab/
        LANGUAGES C CXX)

set(PROJECT_NAME_LOWERCASE opentrim)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option (BUILD_GUI "build gui" ON)
option (BUILD_TESTS "build tests" ON)

message (STATUS "${PROJECT_NAME} version ${PROJECT_VERSION}")

string(TIMESTAMP CMAKE_BUILD_TIME UTC)

message (STATUS "Build time ${CMAKE_BUILD_TIME}")

# !! Add this option to allow the compiler to 
#    optimize for the specific build system
# - Do not use for degugging/code analysis(valgrind)
# - Do not use for package builds
# add_compile_options("-march=native")

# by default, install into $HOME/.local (not /usr/local), so that no root access (and sudo!!) is needed
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if((CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (NOT CMAKE_CROSSCOMPILING))
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}" CACHE PATH "Default install path" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Default install path" FORCE)
  endif()
endif()
message (STATUS "Install prefix =  ${CMAKE_INSTALL_PREFIX}")

# this should be called after setting the prefix !!
include (GNUInstallDirs)

# Get external packages
include(Externals)

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

find_package(HDF5 REQUIRED COMPONENTS C)
include_directories(${HDF5_INCLUDE_DIRS})
message(STATUS "HDF5 include: ${HDF5_INCLUDE_DIRS}")
message(STATUS "HDF5 libs: ${HDF5_LIBRARIES}")

add_subdirectory(source)

if (BUILD_TESTS)
    add_subdirectory(test/scattering_calc)
endif () ## Tests






