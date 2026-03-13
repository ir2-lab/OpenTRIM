# Build opentrim-gui executable

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(GUI_HEADERS
    mainui.h
    simulationoptionsview.h 
    periodictablewidget.h  
    materialdatabasedialog.h
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
    mcdatamodel.h
)

# Collect JSON examples from the top-level examples/json directory.
# CONFIGURE_DEPENDS re-runs cmake if the glob result changes.
file(GLOB _JSON_EXAMPLES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/examples/json/*.json")

# Copy files to the build dir so the generated QRC can reference them with
# a simple relative path (examples/<name>.json).
file(COPY ${_JSON_EXAMPLES}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/examples/")

# Build the QRC content and write it to the binary dir.
set(_EXAMPLES_QRC "${CMAKE_CURRENT_BINARY_DIR}/opentrim_examples.qrc")
set(_qrc "<RCC>\n    <qresource prefix=\"/\">\n")
foreach(_f ${_JSON_EXAMPLES})
    get_filename_component(_fname "${_f}" NAME)
    string(APPEND _qrc "        <file>examples/${_fname}</file>\n")
endforeach()
string(APPEND _qrc "    </qresource>\n</RCC>\n")
file(WRITE "${_EXAMPLES_QRC}" "${_qrc}")

set(GUI_SOURCES
    main.cpp
    mainui.cpp
    opentrim.qrc
    "${_EXAMPLES_QRC}"
    simulationoptionsview.cpp
    materialdatabasedialog.cpp
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
    mcdatamodel.cpp
)

set (GUI_TARGET ${PROJECT_NAME_LOWERCASE}-gui)

add_executable(${GUI_TARGET}
  ${GUI_SOURCES} 
  ${GUI_HEADERS}
)

target_link_libraries(${GUI_TARGET} 
PRIVATE
    Qt5::Widgets
    Qt5::Svg
    Qwt::Qwt
    QtDataBrowser::QtDataBrowser
    ${LIB_TARGET}
    nlohmann_json::nlohmann_json
)

set_target_properties(${GUI_TARGET} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${GUI_TARGET}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # executables (Linux/Windows)
  BUNDLE  DESTINATION ${CMAKE_INSTALL_BINDIR}   # macOS app bundles
)



