set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets DataVisualization)
message(STATUS "Found Qt${QT_VERSION_MAJOR}")

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
    simboxview.h
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
    simboxview.cpp
)

set (GUI_TARGET ${PROJECT_NAME_LOWERCASE}-gui)

add_executable(${GUI_TARGET}
  ${GUI_SOURCES} ${GUI_HEADERS}
  mcplotinfo.h mcplotinfo.cpp
)

target_include_directories(${GUI_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${external_json_SOURCE_DIR}/include/
    ${external_libdedx_SOURCE_DIR}/src/lib
)

target_link_libraries(${GUI_TARGET} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DataVisualization
    QtDataBrowser
    ${LIB_TARGET}
)

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

