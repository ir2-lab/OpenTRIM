# Build the opentrim executable

add_executable(${EXE_TARGET}
    main.cpp
)

# adjust exe name to be the same as the library's
set_property(TARGET ${EXE_TARGET} PROPERTY OUTPUT_NAME ${PROJECT_NAME_LOWERCASE})

target_link_libraries(${EXE_TARGET}
    ${LIB_TARGET}
    CLI11
)

install(TARGETS ${EXE_TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
