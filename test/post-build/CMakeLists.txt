find_program(H5DIFF_EXECUTABLE h5diff REQUIRED)

foreach(N IN ITEMS 1 2 3)

    # Test 1 (fixture setup): run opentrim and produce out${N}.h5 in the build dir
    add_test(NAME PostBuild_RunSim_${N}
        COMMAND "$<TARGET_FILE:opentrim_exe>"
            -f "${CMAKE_CURRENT_SOURCE_DIR}/in${N}.json"
            -o out${N}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    set_tests_properties(PostBuild_RunSim_${N} PROPERTIES
        FIXTURES_SETUP PostBuildFixture_${N}
    )

    # Test 2: compare produced out${N}.h5 against template using h5diff
    add_test(NAME PostBuild_Compare_${N}
        COMMAND "${CMAKE_COMMAND}"
            "-DH5DIFF=${H5DIFF_EXECUTABLE}"
            "-DPRODUCED=${CMAKE_CURRENT_BINARY_DIR}/out${N}.h5"
            "-DTEMPLATE=${CMAKE_CURRENT_SOURCE_DIR}/out${N}.h5"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/compare.cmake"
    )
    set_tests_properties(PostBuild_Compare_${N} PROPERTIES
        FIXTURES_REQUIRED PostBuildFixture_${N}
    )

    # Fixture cleanup: remove produced output after all tests in the fixture finish
    add_test(NAME PostBuild_Cleanup_${N}
        COMMAND "${CMAKE_COMMAND}" -E rm -f "${CMAKE_CURRENT_BINARY_DIR}/out${N}.h5"
    )
    set_tests_properties(PostBuild_Cleanup_${N} PROPERTIES
        FIXTURES_CLEANUP PostBuildFixture_${N}
    )

endforeach()
