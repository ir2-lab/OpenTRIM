cmake_minimum_required(VERSION 3.12.0)
    
project(iradina++ 
        VERSION 0.3.1
        LANGUAGES C CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option (BUILD_IRADINAPP_GUI "build gui" ON)

message (STATUS "iradina++ version ${PROJECT_VERSION}")

string(TIMESTAMP CMAKE_BUILD_TIME UTC)

message (STATUS "Build time ${CMAKE_BUILD_TIME}")

add_compile_options("-march=native")

add_compile_definitions(IRADINAPP_VERSION="${PROJECT_VERSION}"
                        SYSTEM_ID="${CMAKE_CXX_PLATFORM_ID}"
                        COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
                        COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}"
                        BUILD_TIME="${CMAKE_BUILD_TIME}")

include (GNUInstallDirs) 

# by default, install into $HOME/.local (not /usr/local), so that no root access (and sudo!!) is needed
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if((CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (NOT CMAKE_CROSSCOMPILING))
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/iradinapp" CACHE PATH "Default install path" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Default install path" FORCE)
  endif()
endif()

message (STATUS "Install prefix =  ${CMAKE_INSTALL_PREFIX}") 

# use install rpath, no need to use LD_LIBRARY_PATH 
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)

find_package(Eigen3 3.4 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})
message(STATUS "Eigen3 include ${EIGEN3_INCLUDE_DIR}")

find_package(HDF5 REQUIRED COMPONENTS C)
include_directories(${HDF5_INCLUDE_DIRS})
message(STATUS "HDF5 include: ${HDF5_INCLUDE_DIRS}")
message(STATUS "HDF5 libs: ${HDF5_LIBRARIES}")

add_executable(gendedx
    src/gendedx.cpp
    src/elements.h src/elements.cpp)

file(GLOB gendedx_files CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/dedx/*.cpp)

add_library(iondedx SHARED
    src/corteo.h
    src/dedx.h
    src/dedx.cpp
    src/elements.h
    src/elements.cpp
    src/straggling.cpp
    ${gendedx_files})

add_executable(gencorteo
    src/gencorteo.cpp
    src/corteo.h
    src/corteo_xs.h
    src/xs.h)

target_include_directories(gencorteo PRIVATE
    extern/cxxopts-3.1.1/include)

add_custom_command(OUTPUT xs_zbl_data.cpp COMMAND gencorteo -s zbl DEPENDS gencorteo)
add_library(xs_zbl SHARED ${CMAKE_CURRENT_BINARY_DIR}/xs_zbl_data.cpp)
add_custom_command(OUTPUT xs_lj_data.cpp COMMAND gencorteo -s lj DEPENDS gencorteo)
add_library(xs_lj SHARED ${CMAKE_CURRENT_BINARY_DIR}/xs_lj_data.cpp)
add_custom_command(OUTPUT xs_krc_data.cpp COMMAND gencorteo -s krc DEPENDS gencorteo)
add_library(xs_krc SHARED ${CMAKE_CURRENT_BINARY_DIR}/xs_krc_data.cpp)
add_custom_command(OUTPUT xs_moliere_data.cpp COMMAND gencorteo -s moliere DEPENDS gencorteo)
add_library(xs_moliere SHARED ${CMAKE_CURRENT_BINARY_DIR}/xs_moliere_data.cpp)

set (IRADINAPP_HEADERS
    src/dedx.h
    src/corteo.h   
    src/corteo_xs.h
    src/xs.h
    src/elements.h
    src/ion.h 
    src/random_vars.h
    src/target.h 
    src/mccore.h 
    src/geometry.h
    src/ion_beam.h 
    src/arrays.h
    src/event_stream.h 
    src/tally.h 
    src/mcdriver.h 
  )

add_library(iradinapp SHARED
    ${IRADINAPP_HEADERS}
    src/elements.cpp
    src/ion.cpp   
    src/target.cpp
    src/mccore.cpp   
    src/ion_beam.cpp
    src/event_stream.cpp
    src/h5serialize.cpp
    src/tally.cpp
    src/parse_json.cpp    
    src/mcdriver.cpp
)

set(IRADINAPP_LIBS
    "-lm"
    Eigen3::Eigen
    iondedx
    xs_zbl xs_lj xs_krc xs_moliere
    ${HDF5_LIBRARIES})

target_link_libraries(iradinapp ${IRADINAPP_LIBS})

target_include_directories(iradinapp PRIVATE
    extern/json-3.11.3/single_include/ 
    extern/HighFive-2.9.0/include)

add_executable(iradina++
    src/iradina.cpp
)

target_include_directories(iradina++ PRIVATE
    extern/cxxopts-3.1.1/include
    extern/progressbar/include)

target_link_libraries(iradina++
    ${IRADINAPP_LIBS} iradinapp
    )

# add_subdirectory(doc)

set(installable_libs
    iondedx
    xs_zbl xs_lj xs_krc xs_moliere
    iradinapp )

install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES ${IRADINAPP_HEADERS} DESTINATION include)
install(TARGETS iradina++ DESTINATION bin)


if (BUILD_IRADINAPP_GUI)
  add_subdirectory (src/gui)
endif ()





