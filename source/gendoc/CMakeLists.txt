# Build a utility to generate options documentation 
add_executable(genoptionsdoc EXCLUDE_FROM_ALL
    genoptionsdoc.cpp
)

target_link_libraries(genoptionsdoc
PRIVATE
    ${LIB_TARGET}
    nlohmann_json::nlohmann_json
)

add_custom_target(options_doc 
    DEPENDS 
      "${CMAKE_CURRENT_BINARY_DIR}/options.dox.md"
      "${CMAKE_CURRENT_BINARY_DIR}/h5file.dox.md"
)

add_custom_command(OUTPUT options.dox.md h5file.dox.md
    COMMAND genoptionsdoc 
    DEPENDS genoptionsdoc
)