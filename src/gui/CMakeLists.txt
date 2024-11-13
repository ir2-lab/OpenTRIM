set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
message(STATUS "Found Qt${QT_VERSION_MAJOR}")

find_package(Qwt REQUIRED)
message(STATUS "Found Qwt ${QWT_VERSION_STRING} at ${QWT_INCLUDE_DIR}")

set(PROJECT_SOURCES
        main.cpp
        ionsui.cpp
        ionsui.h
        ionsui.qrc
        simulationoptionsview.h simulationoptionsview.cpp
        qjsonpath/qjsonpath.h qjsonpath/qjsonpath.cpp
        periodictablewidget.h periodictablewidget.cpp
        materialsdefview.h materialsdefview.cpp
        periodictable.h periodictable.cpp
        optionsmodel.h optionsmodel.cpp
        floatlineedit.h floatlineedit.cpp
        mydatawidgetmapper.h mydatawidgetmapper.cpp
        targetgeometryview.h targetgeometryview.cpp
        welcomeview.h welcomeview.cpp
        runview.h runview.cpp
        mcdriverobj.h mcdriverobj.cpp
        resultsview.h resultsview.cpp
        qmatplotwidget/src/QMatPlotWidget.h qmatplotwidget/src/QMatPlotWidget.cpp
        jsedit/jsedit.h jsedit/jsedit.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ions-gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(ions-gui
        ${PROJECT_SOURCES}
    )
endif()

target_include_directories(ions-gui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ions-gui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qwt::Qwt
    ${IRADINAPP_LIBS}
    iradinapp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ionsui)
endif()
set_target_properties(ions-gui PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ions-gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

message(STATUS "Install ions-gui at ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ions-gui)
endif()


