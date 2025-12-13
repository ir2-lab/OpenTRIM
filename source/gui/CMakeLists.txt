set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Svg)
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
    value_with_error.h
    jsedit/jsedit.h 
    simboxview.h
    mcplotinfo.h
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
    mcplotinfo.cpp
)

set (GUI_TARGET ${PROJECT_NAME_LOWERCASE}-gui)

add_executable(${GUI_TARGET}
  ${GUI_SOURCES} 
  ${GUI_HEADERS}
)

target_include_directories(${GUI_TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${GUI_TARGET} 
PRIVATE
    Qt5::Widgets
    Qt5::Svg
    Qwt::Qwt
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

