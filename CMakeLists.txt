cmake_minimum_required(VERSION 3.12.0)
    
project(ions 
        VERSION 0.3.2
        DESCRIPTION "Monte-Carlo ion transport simulation"
        HOMEPAGE_URL https://gitlab.com/ir2-lab/
        LANGUAGES C CXX)

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

add_library(dedx SHARED
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

set (PROJECT_HEADERS
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

set (PROJECT_HEADERS_PRIVATE
    src/json_defs_p.h
  )

set (LIB_TARGET ${PROJECT_NAME})
set (EXE_TARGET ${PROJECT_NAME}_exe)

add_library(${LIB_TARGET} SHARED
    ${PROJECT_HEADERS}
    ${PROJECT_HEADERS_PRIVATE}
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

set(PROJECT_LIBS
    "-lm"
    Eigen3::Eigen
    dedx
    xs_zbl xs_lj xs_krc xs_moliere
    ${HDF5_LIBRARIES})

target_link_libraries(${LIB_TARGET} ${PROJECT_LIBS})

target_include_directories(${LIB_TARGET} PRIVATE
    extern/json-3.11.3/single_include/ 
    extern/HighFive-2.9.0/include)

add_executable(${EXE_TARGET}
    src/main.cpp
)
# adjust exe name to be the same as the library's
set_property(TARGET ${EXE_TARGET} PROPERTY OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${EXE_TARGET} PRIVATE
    extern/cxxopts-3.1.1/include
    extern/progressbar/include)

target_link_libraries(${EXE_TARGET}
    ${PROJECT_LIBS} 
    ${LIB_TARGET}
    )

# add_subdirectory(doc)

set(installable_libs
    dedx
    xs_zbl xs_lj xs_krc xs_moliere
    ${LIB_TARGET} )

install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES ${PROJECT_HEADERS} DESTINATION include)
install(TARGETS ${EXE_TARGET} DESTINATION bin)


if (BUILD_GUI)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
message(STATUS "Found Qt${QT_VERSION_MAJOR}")

find_package(Qwt REQUIRED)
message(STATUS "Found Qwt ${QWT_VERSION_STRING} at ${QWT_INCLUDE_DIR}")

set(GUI_SOURCES
    src/gui/main.cpp
    src/gui/ionsui.cpp
    src/gui/ionsui.h
    src/gui/ionsui.qrc
    src/gui/simulationoptionsview.h 
    src/gui/simulationoptionsview.cpp
    src/gui/periodictablewidget.h 
    src/gui/periodictablewidget.cpp
    src/gui/materialsdefview.h 
    src/gui/materialsdefview.cpp
    src/gui/periodictable.h 
    src/gui/periodictable.cpp
    src/gui/optionsmodel.h 
    src/gui/optionsmodel.cpp
    src/gui/floatlineedit.h 
    src/gui/floatlineedit.cpp
    src/gui/mydatawidgetmapper.h 
    src/gui/mydatawidgetmapper.cpp
    src/gui/regionsview.h 
    src/gui/regionsview.cpp
    src/gui/welcomeview.h 
    src/gui/welcomeview.cpp
    src/gui/runview.h 
    src/gui/runview.cpp
    src/gui/mcdriverobj.h 
    src/gui/mcdriverobj.cpp
    src/gui/resultsview.h 
    src/gui/resultsview.cpp
    src/gui/simcontrolwidget.h 
    src/gui/simcontrolwidget.cpp
    src/gui/error_fmt.h
    src/gui/qmatplotwidget/src/QMatPlotWidget.h 
    src/gui/qmatplotwidget/src/QMatPlotWidget.cpp
    src/gui/jsedit/jsedit.h 
    src/gui/jsedit/jsedit.cpp
)

set (GUI_TARGET ${PROJECT_NAME}-gui)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${GUI_TARGET}
        MANUAL_FINALIZATION
        ${GUI_SOURCES}
    )
else()
    add_executable(${GUI_TARGET}
        ${GUI_SOURCES}
    )
endif()

target_include_directories(${GUI_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${GUI_TARGET} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qwt::Qwt
    ${PROJECT_LIBS} 
    ${LIB_TARGET}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ionsui)
endif()

set_target_properties(${GUI_TARGET} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${GUI_TARGET}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

message(STATUS "Install ${GUI_TARGET} at ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${GUI_TARGET})
endif()


endif () ## BUILD GUI





