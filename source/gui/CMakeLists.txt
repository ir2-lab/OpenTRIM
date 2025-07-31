set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
message(STATUS "Found Qt${QT_VERSION_MAJOR}")

find_package(Qwt REQUIRED)
message(STATUS "Found Qwt ${QWT_VERSION_STRING} at ${QWT_INCLUDE_DIR}")

set(GUI_HEADERS
    mainui.h
    simulationoptionsview.h 
    periodictablewidget.h  
    materialsdefview.h 
    optionsmodel.h 
    floatlineedit.h 
    mydatawidgetmapper.h 
    regionsview.h 
    welcomeview.h 
    mcdriverobj.h 
    resultsview.h 
    simcontrolwidget.h 
    tabularview.h
    error_fmt.h
    jsedit/jsedit.h 
    ${external_qmatplotwidget_SOURCE_DIR}/src/QMatPlotWidget.h
)

set(GUI_SOURCES
    main.cpp
    mainui.cpp
    opentrim.qrc
    simulationoptionsview.cpp
    periodictablewidget.cpp
    materialsdefview.cpp
    optionsmodel.cpp
    floatlineedit.cpp
    mydatawidgetmapper.cpp
    regionsview.cpp
    welcomeview.cpp
    mcdriverobj.cpp
    resultsview.cpp
    simcontrolwidget.cpp
    tabularview.cpp
    jsedit/jsedit.cpp
    ${external_qmatplotwidget_SOURCE_DIR}/src/QMatPlotWidget.cpp
)

set (GUI_TARGET ${PROJECT_NAME_LOWERCASE}-gui)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${GUI_TARGET}
        MANUAL_FINALIZATION
        ${GUI_SOURCES} ${GUI_HEADERS}
    )
else()
    add_executable(${GUI_TARGET}
        ${GUI_SOURCES} ${GUI_HEADERS}
    )
endif()

target_include_directories(${GUI_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${external_json_SOURCE_DIR}/single_include/
    ${external_libdedx_SOURCE_DIR}/src/lib
    ${external_qmatplotwidget_SOURCE_DIR}/src
)

target_link_libraries(${GUI_TARGET} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qwt::Qwt
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
    LIBRARY
    RUNTIME
)

message(STATUS "Install ${GUI_TARGET} at ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

if(${QT_VERSION_MAJOR} EQUAL 6)
    qt_finalize_target(${GUI_TARGET})
endif()
